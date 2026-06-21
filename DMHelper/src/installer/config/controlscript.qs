// DMHelper installer/maintenance-tool control script.
//
// Two responsibilities:
//   1. Acquire admin rights up front so the installer registers machine-wide
//      (HKLM) to match its Program Files target, and so the maintenance tool
//      (uninstaller) runs elevated when launched from "Add or Remove Programs".
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
var LOG_FILE_NAME = "dmhelper_install.log";
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

    if (systemInfo.productType === "windows") {
        var gained = installer.gainAdminRights();
        log("gainAdminRights returned " + gained
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
    log("IntroductionPageCallback");
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
