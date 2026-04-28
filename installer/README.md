# Qt Installer Framework Template for AstroCore GCode Sender

This directory contains a template for creating an installer using Qt Installer Framework (QtIFW).

## Structure
- `config.xml`: Main installer configuration
- `packages/com.astrocore.app/meta/package.xml`: Package definition for the main application
- `packages/com.astrocore.app/meta/installscript.qs`: Installation script (creates shortcuts, etc.)
- `packages/com.astrocore.app/data/`: Place application files here (exe, dll, etc.)

## Usage
1. Install Qt Installer Framework (part of Qt).
2. Copy the built application files to `packages/com.astrocore.app/data/` (AstroCore.exe, all DLLs, translations folder, etc.).
3. Run the following command from the installer directory:
   ```
   binarycreator.exe -c config.xml -p packages AstroCore-Installer.exe
   ```
   This creates a standalone offline installer.

## Customization
- Update version numbers in `config.xml` and `packages/com.astrocore.app/meta/package.xml`
- Modify `installscript.qs` to customize installation behavior
- Add installer icons/images (update paths in config.xml)
- For online installers, use `repogen` to create a repository

Refer to QtIFW documentation for advanced features.