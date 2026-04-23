---
title: Help and Bug Reports
---

# Help and Bug Reports

If you have problems with AstroCore, found a bug or want to request a feature, this page explains how to do it.

## Where to report

The main place for bug reports and feature requests is the GitHub issue tracker:

- Issues: https://github.com/etet100/AstroCore-GCode-Sender/issues

Before creating a new issue, quickly check if a similar issue already exists.

## What to include in a bug report

To make it easier to reproduce and fix a problem, please include:

- AstroCore version (for example from the release name or About dialog)
- Operating system (Windows / Linux, version and 32/64 bit)
- CNC controller firmware and version (GRBL, uCNC, grblHAL, FluidNC, etc.)
- Connection mode (serial, TCP, virtual mode)
- Exact steps to reproduce the problem
- Expected result and actual result
- Screenshots or short screen recording if possible

For crashes or serious errors, it is also useful to attach log files.

## Log files

You can enable logging to file with command line options:

```text
G-Pilot.exe --log-to-file --trim-log
```

This creates or clears a `GPilot.log` file at startup and writes debug information into it. When reporting a bug, you can attach this file to the issue.

## Questions and discussions

For general questions, ideas and design discussions you can also use GitHub Discussions (if enabled) or open an issue tagged as a question.

Polite and clear descriptions help a lot. Short, focused issues are easier to handle than very long ones that mix many topics.
