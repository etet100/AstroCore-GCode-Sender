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
            "@TargetDir@/AstroCore.exe",
            "@StartMenuDir@/AstroCore.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/AstroCore.exe",
            "iconId=0",
            "description=Launch AstroCore GCode Sender");

        // Create Desktop shortcut (optional)
        component.addOperation("CreateShortcut",
            "@TargetDir@/AstroCore.exe",
            "@DesktopDir@/AstroCore.lnk",
            "workingDirectory=@TargetDir@",
            "iconPath=@TargetDir@/AstroCore.exe",
            "iconId=0",
            "description=Launch AstroCore GCode Sender");
    }
}

Component.prototype.createOperationsForArchive = function(archive)
{
    // Called for each archive during installation
    component.addOperation("Extract", archive, "@TargetDir@");
}