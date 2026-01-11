function Controller() {
}

Controller.prototype.ComponentSelectionPageCallback = function() {
    var targetDir = installer.value("TargetDir");
    // Ensure standard paths
    targetDir = targetDir.replace(/\\/g, "/");
    var maintenanceTool = targetDir + "/maintenancetool.exe";

    if (installer.fileExists(maintenanceTool)) {
         var result = QMessageBox.question("quit.question", "G-Pilot Installer",
             "The selected directory is not empty.\nIf you continue, files in it will be overwritten.\n\nDo you want to continue?",
             QMessageBox.Yes | QMessageBox.No);

         if (result == QMessageBox.No) {
             gui.clickButton(buttons.BackButton);
         }
    }
}
