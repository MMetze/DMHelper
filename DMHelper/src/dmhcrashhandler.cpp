#include "dmhcrashhandler.h"
#include <QString>
#include <QDir>
#include <QDebug>

#ifdef Q_OS_WIN

#include <windows.h>
#include <dbghelp.h>
#include <strsafe.h>

namespace
{
    // Maximum length of the cached dump directory. Resolved once at install
    // time so the exception filter itself performs no Qt allocations.
    constexpr int DUMP_DIR_BUFFER_SIZE = MAX_PATH;

    // Minidump content selection. MiniDumpWithThreadInfo on top of a normal
    // dump captures every thread's stack and context plus the module list -
    // enough to symbolicate a libVLC-thread crash. We deliberately avoid the
    // heavier MiniDumpWithFullMemoryInfo / MiniDumpWithIndirectlyReferencedMemory
    // flags: they make MiniDumpWriteDump far more likely to fail (and emit a
    // zero-byte file) when the process heap is already damaged by the crash.
    constexpr MINIDUMP_TYPE DUMP_TYPE = static_cast<MINIDUMP_TYPE>(
        MiniDumpNormal |
        MiniDumpWithThreadInfo |
        MiniDumpWithUnloadedModules);

    // How long the faulting thread waits for the dedicated writer thread to
    // finish before giving up and letting the process die.
    constexpr DWORD DUMP_WRITE_TIMEOUT_MS = 60000;

    wchar_t g_dumpDir[DUMP_DIR_BUFFER_SIZE] = {0};
    LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = nullptr;

    // Shared state handed from the faulting thread to the writer thread.
    EXCEPTION_POINTERS* g_exceptionPointers = nullptr;
    DWORD g_faultingThreadId = 0;
    HANDLE g_dumpRequestedEvent = nullptr;
    HANDLE g_dumpCompletedEvent = nullptr;
    HANDLE g_dumpThread = nullptr;

    // Writes a small ASCII sidecar describing the dump outcome. Uses only Win32
    // calls (no CRT heap) so it is safe to run from a crashed process.
    void writeStatusSidecar(const wchar_t* dumpPath, BOOL dumpResult, DWORD lastError, LONGLONG dumpSize)
    {
        wchar_t statusPath[DUMP_DIR_BUFFER_SIZE];
        if(FAILED(StringCchPrintfW(statusPath, DUMP_DIR_BUFFER_SIZE, L"%s.txt", dumpPath)))
            return;

        HANDLE statusFile = CreateFileW(statusPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if(statusFile == INVALID_HANDLE_VALUE)
            return;

        char buffer[256];
        int written = wsprintfA(buffer,
                                "MiniDumpWriteDump result=%d GetLastError=%lu dumpBytes=%lld\r\n",
                                dumpResult ? 1 : 0,
                                lastError,
                                dumpSize);
        if(written > 0)
        {
            DWORD bytesWritten = 0;
            WriteFile(statusFile, buffer, static_cast<DWORD>(written), &bytesWritten, nullptr);
        }

        CloseHandle(statusFile);
    }

    void writeDump()
    {
        if(g_dumpDir[0] == L'\0')
            return;

        SYSTEMTIME st;
        GetLocalTime(&st);

        wchar_t dumpPath[DUMP_DIR_BUFFER_SIZE];
        if(FAILED(StringCchPrintfW(dumpPath,
                                   DUMP_DIR_BUFFER_SIZE,
                                   L"%s\\DMHelper-%04d-%02d-%02d-%02d-%02d-%02d.dmp",
                                   g_dumpDir,
                                   st.wYear, st.wMonth, st.wDay,
                                   st.wHour, st.wMinute, st.wSecond)))
            return;

        HANDLE dumpFile = CreateFileW(dumpPath,
                                      GENERIC_WRITE,
                                      0,
                                      nullptr,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      nullptr);
        if(dumpFile == INVALID_HANDLE_VALUE)
            return;

        MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;
        dumpExceptionInfo.ThreadId = g_faultingThreadId;
        dumpExceptionInfo.ExceptionPointers = g_exceptionPointers;
        dumpExceptionInfo.ClientPointers = FALSE;

        BOOL dumpResult = FALSE;
        DWORD lastError = 0;

        // MiniDumpWriteDump can itself fault on a badly corrupted process;
        // guard it so we still flush, size and report the result.
        __try
        {
            dumpResult = MiniDumpWriteDump(GetCurrentProcess(),
                                           GetCurrentProcessId(),
                                           dumpFile,
                                           DUMP_TYPE,
                                           (g_exceptionPointers ? &dumpExceptionInfo : nullptr),
                                           nullptr,
                                           nullptr);
            if(!dumpResult)
                lastError = GetLastError();
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            dumpResult = FALSE;
            lastError = static_cast<DWORD>(GetExceptionCode());
        }

        FlushFileBuffers(dumpFile);

        LARGE_INTEGER dumpSize;
        dumpSize.QuadPart = 0;
        GetFileSizeEx(dumpFile, &dumpSize);

        CloseHandle(dumpFile);

        writeStatusSidecar(dumpPath, dumpResult, lastError, dumpSize.QuadPart);
    }

    // Parked writer thread. Running the dump from a dedicated thread keeps it
    // off the (possibly unusable) faulting thread stack and is the standard
    // robust pattern for in-process minidumps.
    DWORD WINAPI dumpThreadProc(LPVOID)
    {
        for(;;)
        {
            if(WaitForSingleObject(g_dumpRequestedEvent, INFINITE) != WAIT_OBJECT_0)
                return 0;

            writeDump();
            SetEvent(g_dumpCompletedEvent);
        }
    }

    LONG WINAPI dmhUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
    {
        if((g_dumpThread) && (g_dumpRequestedEvent) && (g_dumpCompletedEvent))
        {
            g_exceptionPointers = exceptionInfo;
            g_faultingThreadId = GetCurrentThreadId();

            SetEvent(g_dumpRequestedEvent);
            WaitForSingleObject(g_dumpCompletedEvent, DUMP_WRITE_TIMEOUT_MS);
        }

        if(g_previousFilter)
            return g_previousFilter(exceptionInfo);

        return EXCEPTION_EXECUTE_HANDLER;
    }
}

void DMHCrashHandler::install(const QString& dumpDirPath)
{
    QDir().mkpath(dumpDirPath);

    const QString nativeDir = QDir::toNativeSeparators(dumpDirPath);
    const int copied = nativeDir.toWCharArray(g_dumpDir);
    g_dumpDir[qBound(0, copied, DUMP_DIR_BUFFER_SIZE - 1)] = L'\0';

    g_dumpRequestedEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    g_dumpCompletedEvent = CreateEventW(nullptr, FALSE, FALSE, nullptr);
    if((g_dumpRequestedEvent) && (g_dumpCompletedEvent))
        g_dumpThread = CreateThread(nullptr, 0, dumpThreadProc, nullptr, 0, nullptr);

    if(!g_dumpThread)
    {
        qWarning() << "[DMHCrashHandler] Failed to create dump writer thread; crash dumps disabled";
        return;
    }

    g_previousFilter = SetUnhandledExceptionFilter(dmhUnhandledExceptionFilter);

    qInfo() << "[DMHCrashHandler] Crash handler installed, dumps will be written to" << nativeDir;
}

#else

void DMHCrashHandler::install(const QString& dumpDirPath)
{
    Q_UNUSED(dumpDirPath);
}

#endif
