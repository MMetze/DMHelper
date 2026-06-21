// DMHelper installer/maintenance-tool control script.
//
// Two responsibilities:
//   1. Elevation. The installer acquires admin rights up front so it registers
//      machine-wide (HKLM) to match its Program Files target. The maintenance
//      tool (uninstaller) does NOT elevate in place: when launched from "Add or
//      Remove Programs" the host runs it inside a job object and terminates the
//      visible UI process right after the first page (confirmed: exit code 1 =
//      TerminateProcess, not a crash). Instead it relaunches itself elevated
//      via ShellExecute/RunAs, which the UAC service spawns OUTSIDE the host's
//      job object, so the surviving UI process is no longer killed. See the
//      IntroductionPageCallback.
//   2. Write a verbose log of the install/uninstall lifecycle to a file, so the
//      "flash and close" uninstall behaviour can be diagnosed from the field.
//
// Preferred log location: %APPDATA%\DMHelper\log
// Fallback (if APPDATA is unavailable): <TargetDir>\log
//
// The log is buffered in memory and the complete buffer is rewritten on every
// flush. We deliberately avoid the QtIFW "AppendFile" operation: it creates a
// timestamped backup of the file (for its UNDO step) on every call, which
// littered the log directory with one "<name>.<random>" file per write.

var LOG_DIR_NAME = "DMHelper\\log";

// One log file per run, stamped with the start time (UTC, matching the in-file
// timestamps), e.g. "dmhelper_install-2026-06-21-22-10-26.log". Computed once
// when the script is loaded so every flush in this process targets the same
// file (a single, complete file per install/uninstall run).
function startTimestamp()
{
    var d = new Date();
    function p(n) { return (n < 10 ? "0" : "") + n; }
    return d.getUTCFullYear() + "-" + p(d.getUTCMonth() + 1) + "-" + p(d.getUTCDate())
        + "-" + p(d.getUTCHours()) + "-" + p(d.getUTCMinutes()) + "-" + p(d.getUTCSeconds());
}

var LOG_FILE_NAME = "dmhelper_install-" + startTimestamp() + ".log";
var logBuffer = [];

function logFilePath()
{
    var appData = installer.environmentVariable("APPDATA");
    if (appData && appData.length > 0)
        return appData + "\\" + LOG_DIR_NAME + "\\" + LOG_FILE_NAME;

    var targetDir = installer.value("TargetDir");
    if (targetDir && targetDir.length > 0)
        return installer.toNativeSeparators(targetDir) + "\\log\\" + LOG_FILE_NAME;

    return "";
}

function flushLog()
{
    if (systemInfo.productType !== "windows")
        return;

    var file = logFilePath();
    if (!file || file.length === 0)
        return;

    // Rewrite the whole buffer in one process. The content is piped over stdin
    // so it needs no shell escaping; only the path is embedded, and the install
    // path never contains a single quote.
    var content = logBuffer.join("\r\n") + "\r\n";
    var psCommand =
        "$p = '" + file + "'; " +
        "New-Item -ItemType Directory -Force -Path (Split-Path -Parent $p) | Out-Null; " +
        "$input | Set-Content -LiteralPath $p -Encoding UTF8";

    installer.execute("powershell",
        ["-NoProfile", "-NonInteractive", "-WindowStyle", "Hidden", "-Command", psCommand],
        content);
}

function log(message)
{
    // Always emit to the console too (visible with --verbose).
    console.log("DMHelper installer: " + message);

    logBuffer.push("[" + new Date().toISOString() + "] " + message);

    try {
        flushLog();
    } catch (e) {
        console.log("DMHelper installer: log write failed: " + e);
    }
}

function Controller()
{
    log("==================== control script start ====================");
    log("mode: isInstaller=" + installer.isInstaller()
        + " isUninstaller=" + installer.isUninstaller()
        + " isUpdater=" + installer.isUpdater()
        + " isPackageManager=" + installer.isPackageManager());
    log("hasAdminRights (before)=" + installer.hasAdminRights());

    // The installer is double-clicked by the user, so it can elevate in place
    // here (this path is known-good: install completes and registers HKLM).
    //
    // The maintenance tool (uninstaller) is NOT elevated here. When launched
    // from "Add or Remove Programs", the host runs us inside a job object and
    // calls TerminateProcess on the visible UI process right after the first
    // page (confirmed: exit code 1, no crash, no error dialog). Elevating in
    // place keeps that doomed UI process as the live one. Instead, the
    // IntroductionPageCallback relaunches the tool elevated via ShellExecute
    // (RunAs); the UAC service spawns the new process OUTSIDE the host's job
    // object, so it survives. See IntroductionPageCallback below.
    if (systemInfo.productType === "windows" && installer.isInstaller()) {
        var gained = installer.gainAdminRights();
        log("installer gainAdminRights returned " + gained
            + ", hasAdminRights (after)=" + installer.hasAdminRights());
    }

    installer.installationStarted.connect(function() { log("installationStarted"); });
    installer.installationFinished.connect(function() { log("installationFinished"); });
    installer.installationInterrupted.connect(function() { log("installationInterrupted"); });
    installer.uninstallationStarted.connect(function() { log("uninstallationStarted"); });
    installer.uninstallationFinished.connect(function() { log("uninstallationFinished"); });
}

Controller.prototype.IntroductionPageCallback = function()
{
    log("IntroductionPageCallback hasAdminRights=" + installer.hasAdminRights());

    // Only the Windows maintenance tool needs the out-of-job relaunch.
    if (systemInfo.productType !== "windows" || !installer.isUninstaller())
        return;

    // The elevated relaunch starts already running with admin rights, so this
    // branch is the surviving, out-of-job instance: just proceed normally.
    // This is also the guard that prevents an infinite relaunch loop.
    if (installer.hasAdminRights()) {
        log("running elevated and out-of-job; proceeding with maintenance UI");
        return;
    }

    // Not elevated => this is the in-job instance launched by the Add/Remove
    // host, which will TerminateProcess us in a moment. Relaunch the tool
    // elevated and detached: powershell's Start-Process -Verb RunAs routes
    // through the UAC service, which spawns uninstaller.exe outside this job
    // object so it is not killed. Then cancel this (doomed) instance.
    var targetDir = installer.value("TargetDir");
    var tool = installer.toNativeSeparators(targetDir) + "\\uninstaller.exe";
    log("in-job instance detected; relaunching elevated/detached: " + tool);

    var ok = installer.executeDetached("powershell",
        ["-NoProfile", "-NonInteractive", "-WindowStyle", "Hidden", "-Command",
         "Start-Process -FilePath '" + tool + "' -Verb RunAs"],
        targetDir);
    log("elevated relaunch started=" + ok + "; cancelling in-job instance");

    gui.clickButton(buttons.CancelButton);
};

Controller.prototype.TargetDirectoryPageCallback = function()
{
    log("TargetDirectoryPageCallback targetDir=" + installer.value("TargetDir"));
};

Controller.prototype.ReadyForInstallationPageCallback = function()
{
    log("ReadyForInstallationPageCallback");
};

Controller.prototype.PerformInstallationPageCallback = function()
{
    log("PerformInstallationPageCallback");
};

Controller.prototype.FinishedPageCallback = function()
{
    log("FinishedPageCallback");
};
