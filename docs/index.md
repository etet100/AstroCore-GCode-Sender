---
title: AstroCore G-Code Sender
---

# AstroCore G-Code Sender

What AstroCore stands for?

- **Astronaut's Core**: Like an astronaut at mission control, AstroCore puts you in the cockpit of your CNC machine with every gauge and button right at hand.
- **Astronomical Core**: Your CNC work deserves astronomical precision. AstroCore is the core engine that keeps every cut, move and probe on target.
- **Astro + Core**: "Astro" brings the reach of the stars — modern, scalable, ready for any supported firmware. "Core" is the central control unit that ties every module together.
- **Stellar Core**: Like the core of a star that powers everything around it, AstroCore is the beating heart of your workshop.
- **A Smart, Trusted, Reliable, Open Core**: A nerdy backronym we stand by — free, open source, and smart in the details.

*This fork is based on the Candle `experimental` branch. The main goal is to add joystick/joypad support. Other than that, I'm making improvements/bugfixes at my discretion.*

*Any help is welcome!*

## What is AstroCore?

GRBL/uCNC/FluidNC controller application with G-Code visualizer written in Qt.

Supported functions:
* Controlling GRBL-based cnc-machine via console commands, buttons on form, numpad.
* Monitoring cnc-machine state.
* Loading, editing, saving and sending of G-code files to cnc-machine.
* Visualizing G-code files.
* Camera.
* Joystick/Joypad/Controller support.
* Customizable interface.
* uCNC/grblHAL/FluidNC virtual modes (cnc machine simulator).
* Automatic checking for new releases.

## Documentation

Main documentation pages available in this folder:

- [About AstroCore](about.md)
- [User interface](ui.md)
- [Jogging](jogging.md)
- [G-code editing](gcode-editing.md)
- [Heightmap / Surface leveling](heightmap.md)
- [Connection and virtual modes](connection-modes.md)
- [Checking for updates](updates.md)
- [Log browser](log-browser.md)
- [Architecture overview](architecture.md)
- [Application states](application-states.md)
- [Configuration system](configuration.md)
- [Build and packaging](build.md)
- [Command line options](command-line.md)
- [Screenshots](screenshots.md)
- [Help and bug reports](help.md)
- [Testing and examples](testing.md)
