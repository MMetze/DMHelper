// DMHelper installer/maintenance-tool control script.
//
// Two responsibilities:
//   1. Acquire admin rights up front so the installer registers machine-wide
//      (HKLM) to match its Program Files target, and so the maintenance tool
//      (uninstaller) runs elevated when launched from "Add or Remove Programs".
//   2. Write a verbose log of the install/uninstall lifecycle to a file, so the
//      "flash and close" uninstall behaviour can be diagnosed from the field.
//
// Preferred log location: %APPDATA%\DMHelper\logs
// Fallback (if APPDATA is unavailable): <TargetDir>\logs

var LOG_DIR_NAME = "DMHelper\\logs";
var LOG_FILE_NAME = "dmhelper_install.log";

function logDirectory()
{
    var appData = installer.environmentVariable("APPDATA");
    if (appData && appData.length > 0)
        return appData + "\\" + LOG_DIR_NAME;

    var targetDir = installer.value("TargetDir");
    if (targetDir && targetDir.length > 0)
        return installer.toNativeSeparators(targetDir) + "\\logs";

    return "";
}

function log(message)
{
    // Always emit to the console too (visible with --verbose).
    console.log("DMHelper installer: " + message);

    try {
        var dir = logDirectory();
        if (!dir || dir.length === 0)
            return;

        var line = "[" + new Date().toISOString() + "] " + message + "\n";
        installer.performOperation("Mkdir", [dir]);
        installer.performOperation("AppendFile", [dir + "\\" + LOG_FILE_NAME, line]);
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
