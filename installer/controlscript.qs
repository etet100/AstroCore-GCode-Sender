function Controller()
{
}

Controller.prototype.TargetDirectoryPageLeaving = function()
{
    var targetDir = installer.value("TargetDir");
    var maintenanceTool = targetDir + "/maintenancetool.exe";
    var componentsXml = targetDir + "/components.xml";

    // Check if maintenancetool exists (which blocks installation)
    if (installer.fileExists(maintenanceTool) || installer.fileExists(componentsXml)) {
        var result = QMessageBox.question("overwrite.question", "G-Pilot Installer",
            "An existing installation has been detected in the selected directory.\nDo you want to overwrite it? (This will remove information about the previous installation)",
            QMessageBox.Yes | QMessageBox.No);

        if (result == QMessageBox.Yes) {
            // We remove files that make the installer think it is an existing installation
            // This allows the installer to install in the same folder (overwriting files)
            var files = [
                maintenanceTool,
                targetDir + "/maintenancetool.ini",
                targetDir + "/maintenancetool.dat",
                componentsXml,
                targetDir + "/network.xml",
                targetDir + "/InstallationLog.txt"
            ];

            if (systemInfo.productType === "windows") {
                for (var i = 0; i < files.length; i++) {
                    var file = files[i];
                    if (installer.fileExists(file)) {
                        installer.execute("cmd", ["/c", "del", "/f", "/q", file.replace(/\//g, "\\")]);
                    }
                }
            } else {
                 for (var i = 0; i < files.length; i++) {
                    var file = files[i];
                    if (installer.fileExists(file)) {
                        installer.execute("rm", ["-f", file]);
                    }
                }
            }
        }
    }
}
