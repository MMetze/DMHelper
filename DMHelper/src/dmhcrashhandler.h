#ifndef DMHCRASHHANDLER_H
#define DMHCRASHHANDLER_H

class QString;

// Installs a process-wide handler that writes a minidump (.dmp) when the
// application crashes with an otherwise-unhandled exception (e.g. an access
// violation raised from libVLC's decode/render threads). The dump is written
// to the supplied directory using the same naming convention as the log files
// (DMHelper-yyyy-MM-dd-hh-mm-ss.dmp).
//
// On non-Windows platforms this is currently a no-op.
namespace DMHCrashHandler
{
    void install(const QString& dumpDirPath);
}

#endif // DMHCRASHHANDLER_H
