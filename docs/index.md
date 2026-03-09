---
title: G-Pilot G-Code Sender
---

# G-Pilot G-Code Sender

GRBL/uCNC/FluidNC controller application with G-Code visualizer written in Qt.

G-Pilot is a fork of the Candle experimental branch. The main goal is to add joystick/joypad support and improve the application step by step.

## Key features

- Control GRBL/uCNC/FluidNC-based CNC machines using console, buttons, and numpad
- Monitor machine state in real time
- Load, edit, save and stream G-code files
- Built-in G-code visualizer
- Camera support
- Joystick / joypad / controller support
- Customizable user interface with scaling
- Virtual modes for uCNC, grblHAL and FluidNC (no real hardware required)

## Download & install

Automatic builds (portable zip and installer) are available as prereleases:

- Latest version: https://github.com/etet100/G-Pilot-GCode-Sender/releases/latest
- Debug builds – with extra logging, useful for testing and bug reports
- Release builds – optimized for everyday use

> Note: builds are for testing and preview. Feedback and bug reports are welcome.

## System requirements

- Windows 10 or Linux x86 (Linux not fully tested)
- OpenGL 3.0 capable GPU
- ~150 MB free disk space

## Build from source (Windows, Qt, MinGW/LLVM)

Basic steps:

1. Clone repository with submodules:
	```
	git clone --recurse-submodules https://github.com/etet100/G-Pilot-GCode-Sender
	cd G-Pilot-GCode-Sender
	git submodule update --init --recursive
	```
2. Open `gpilot.pro` in Qt Creator
3. Use Qt 6.8 with LLVM/Clang or MinGW 64-bit (MSVC is not supported)
4. Build the project – binaries will appear in the `bin` directory

More detailed build instructions will be available on a separate page.

## Documentation

Main documentation pages available in this folder:

- [About G-Pilot](about.md)
- [User interface](ui.md)
- [G-code editing](gcode-editing.md)
- [Connection and virtual modes](connection-modes.md)
- [Architecture overview](architecture.md)
- [Application states](application-states.md)
- [Configuration system](configuration.md)
- [Build and packaging](build.md)
- [Command line options](command-line.md)
- [Screenshots](screenshots.md)
 - [Help and bug reports](help.md)
 - [Testing and examples](testing.md)
