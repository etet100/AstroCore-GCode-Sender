# Qt Installer Framework Template for G-Pilot

This directory contains a template for creating an installer using Qt Installer Framework (QtIFW).

## Structure
- `config.xml`: Main installer configuration
- `packages/com.gpilot.app/meta/package.xml`: Package definition for the main application
- `packages/com.gpilot.app/meta/installscript.qs`: Installation script (creates shortcuts, etc.)
- `packages/com.gpilot.app/data/`: Place application files here (exe, dll, etc.)

## Usage
1. Install Qt Installer Framework (part of Qt).
2. Copy the built application files to `packages/com.gpilot.app/data/` (GPilot.exe, all DLLs, translations folder, etc.).
3. Run the following command from the installer directory:
   ```
   binarycreator.exe -c config.xml -p packages G-Pilot-Installer.exe
   ```
   This creates a standalone offline installer.

## Customization
- Update version numbers in `config.xml` and `packages/com.gpilot.app/meta/package.xml`
- Modify `installscript.qs` to customize installation behavior
- Add installer icons/images (update paths in config.xml)
- For online installers, use `repogen` to create a repository

Refer to QtIFW documentation for advanced features.