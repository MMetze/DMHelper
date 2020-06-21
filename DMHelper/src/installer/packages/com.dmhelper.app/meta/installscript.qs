function Component()
{
    // Access the global component's name property and log it to the debug console.
    console.log("component: " + component.displayName);
}

Component.prototype.createOperations = function()
{
    // Call the default implementation
    component.createOperations();

    if (systemInfo.productType === "windows") {
        // return value 3010 means it need a reboot, but in most cases that is not needed for running Qt application
        // return value 5100 means there's a newer version of the runtime already installed
        // 1602: Happens when the installation of Redistributable Package is cancelled.
        // 1638: Happens when another version of the Redistributable Package is already installed.
        component.addElevatedOperation("Execute", "{0,3010,1602,1638,5100}", "@TargetDir@\\vc_redist.x64.exe", "/norestart", "/q");

        component.addOperation("CreateShortcut", "@TargetDir@/DMHelper.exe", "@StartMenuDir@/DM Helper.lnk",
            "workingDirectory=@TargetDir@", "description=Open the DM Helper");

        component.addOperation("CreateShortcut", "@TargetDir@/maintenancetool.exe", "@StartMenuDir@/Uninstall DM Helper.lnk",
            "workingDirectory=@TargetDir@", "description=Open the DM Helper maintenance tool");
    }
}
