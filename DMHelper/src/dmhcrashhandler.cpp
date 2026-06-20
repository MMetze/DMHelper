#include "dmhcrashhandler.h"
#include <QString>
#include <QDir>
#include <QDebug>

#ifdef Q_OS_WIN

#include <windows.h>
#include <dbghelp.h>

namespace
{
    // Maximum length of the cached dump directory. Resolved once at install
    // time so the exception filter itself performs no Qt allocations.
    constexpr int DUMP_DIR_BUFFER_SIZE = MAX_PATH;

    // Minidump content selection. Captures every thread's stack and the memory
    // their registers reference, the module list (loaded and unloaded) and
    // process/handle metadata - enough to symbolicate a libVLC-thread crash
    // without writing the entire process heap.
    constexpr MINIDUMP_TYPE DUMP_TYPE = static_cast<MINIDUMP_TYPE>(
        MiniDumpWithDataSegs |
        MiniDumpWithThreadInfo |
        MiniDumpWithIndirectlyReferencedMemory |
        MiniDumpWithFullMemoryInfo |
        MiniDumpWithUnloadedModules);

    wchar_t g_dumpDir[DUMP_DIR_BUFFER_SIZE] = {0};
    LPTOP_LEVEL_EXCEPTION_FILTER g_previousFilter = nullptr;

    LONG WINAPI dmhUnhandledExceptionFilter(EXCEPTION_POINTERS* exceptionInfo)
    {
        if(g_dumpDir[0] == L'\0')
            return EXCEPTION_CONTINUE_SEARCH;

        SYSTEMTIME st;
        GetLocalTime(&st);

        wchar_t dumpPath[DUMP_DIR_BUFFER_SIZE];
        swprintf_s(dumpPath,
                   L"%s\\DMHelper-%04d-%02d-%02d-%02d-%02d-%02d.dmp",
                   g_dumpDir,
                   st.wYear, st.wMonth, st.wDay,
                   st.wHour, st.wMinute, st.wSecond);

        HANDLE dumpFile = CreateFileW(dumpPath,
                                      GENERIC_WRITE,
                                      0,
                                      nullptr,
                                      CREATE_ALWAYS,
                                      FILE_ATTRIBUTE_NORMAL,
                                      nullptr);
        if(dumpFile != INVALID_HANDLE_VALUE)
        {
            MINIDUMP_EXCEPTION_INFORMATION dumpExceptionInfo;
            dumpExceptionInfo.ThreadId = GetCurrentThreadId();
            dumpExceptionInfo.ExceptionPointers = exceptionInfo;
            dumpExceptionInfo.ClientPointers = FALSE;

            MiniDumpWriteDump(GetCurrentProcess(),
                              GetCurrentProcessId(),
                              dumpFile,
                              DUMP_TYPE,
                              &dumpExceptionInfo,
                              nullptr,
                              nullptr);

            CloseHandle(dumpFile);
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

    g_previousFilter = SetUnhandledExceptionFilter(dmhUnhandledExceptionFilter);

    qInfo() << "[DMHCrashHandler] Crash handler installed, dumps will be written to" << nativeDir;
}

#else

void DMHCrashHandler::install(const QString& dumpDirPath)
{
    Q_UNUSED(dumpDirPath);
}

#endif
