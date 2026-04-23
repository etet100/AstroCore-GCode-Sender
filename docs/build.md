---
title: Build and Packaging
---

# Build and Packaging

This page describes how to build AstroCore from source and how to build Qt Creator designer plugins.

## Build requirements

- Qt 6.8 with LLVM/Clang compiler (recommended) or MinGW/GCC 64-bit
- MSVC compiler is not officially supported and may not work correctly

> Note: CMake files in the repository are not maintained. The supported build system is QMake (`gpilot.pro`).

## Build with Qt Creator (recommended)

1. Clone the repository with submodules:
   ```
   git clone --recurse-submodules https://github.com/etet100/AstroCore-GCode-Sender
   cd AstroCore-GCode-Sender
   git submodule update --init --recursive
   ```
2. Open `gpilot.pro` in Qt Creator.
3. Select Qt 6.x and LLVM/Clang (recommended) or MinGW 64-bit.
4. (Optional) Enable multi-threaded compilation:
   - Projects → Build → Build Steps → Make → Make arguments
   - Add `-j8` (adjust to your CPU core count)
5. (Optional) Configure auto-copy of DLL files after build:
   - Projects → Build or Deploy Settings → Build Steps → Custom Process Step
   - Command: `python`
   - Arguments: `copy_files.py %{ActiveProject:BuildConfig:Path} %{ActiveProject:BuildConfig:Path}\src\gpilot\`
   - Working directory: `%{ActiveProject:ProjectDirectory}\scripts`
6. Build the project (Ctrl+B).
7. Executable and DLL files will appear in the `bin` folder.

## Build with qmake from command line

1. Add Qt bin directory to the PATH:
   ```
   set PATH=C:\Qt\6.8.1\llvm-mingw_64\bin;%PATH%
   ```
2. Run qmake to generate Makefile:
   ```
   qmake gpilot.pro
   ```
3. Build the project (LLVM/Clang or MinGW):
   ```
   mingw32-make -j8
   ```
4. The executable and DLL files will be generated in the `bin` folder.

## Packaging

- Copy files from the `bin` directory (exe, dll, translations, LICENSE).
- Make sure that all required DLLs are present (Qt and vendor DLLs).

## Qt Creator designer plugins

AstroCore includes custom Qt Designer widgets that can be used in Qt Creator. These plugins must be built with the same compiler that was used for Qt Creator (typically MSVC 2022 64-bit).

> Important: The main AstroCore project uses a different compiler (LLVM/Clang or MinGW). Designer plugins are built separately.

### Steps

1. In Qt Creator open `src/designerplugins/designerplugins.pro`.
2. Select Qt 6.x with MSVC 2022 64-bit kit (same as Qt Creator).
3. Build **Release** configuration (Qt Creator cannot load Debug plugins).
4. Configure install path, for example:
   ```
   qmake QTCREATOR_PLUGINS_PATH="C:/YourPath/QtCreator/bin/plugins/designer"
   ```
   or set environment variable:
   ```
   set QTCREATOR_PLUGINS_PATH=C:/YourPath/QtCreator/bin/plugins/designer
   ```
5. Build and install:
   - Build project (Ctrl+B)
   - Run `make install` or add an install step in Qt Creator
6. Restart Qt Creator. Custom widgets (for example ColorPicker, Slider, SliderBox, StyledToolButton) will appear in the Designer widget palette.
