function Component()
{
    // Constructor
}

Component.prototype.createOperations = function()
{
    // Call default implementation to actually install the application
    component.createOperations();

    if (systemInfo.productType === "windows") {
        // Create Start Menu shortcuts
        component.addOperation("CreateShortcut",
            "@TargetDir@/GPilot.exe",
            "@StartMenuDir@/G-Pilot.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/GPilot.exe",
            "iconId=0",
            "description=Launch G-Pilot CNC Controller");

        // Create Desktop shortcut (optional)
        component.addOperation("CreateShortcut",
            "@TargetDir@/GPilot.exe",
            "@DesktopDir@/G-Pilot.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/GPilot.exe",
            "iconId=0",
            "description=Launch G-Pilot CNC Controller");
    }
}

Component.prototype.createOperationsForArchive = function(archive)
{
    // Called for each archive during installation
    component.addOperation("Extract", archive, "@TargetDir@");
}