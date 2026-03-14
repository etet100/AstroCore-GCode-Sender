---
title: Command Line Options
---

# Command Line Options

G-Pilot supports several command line switches that control logging, configuration format and console behavior.

## Available options

- `-l` or `--log-to-file` – Enable logging debug information to `GPilot.log` file.
- `-t` or `--trim-log` – Clear `GPilot.log` file at startup (use together with log-to-file).
- `-c` or `--config-type <type>` – Select configuration file format. Available types: `ini`, `json`, `xml`. Default: `ini`.
- `-co` or `--console` – Open G-Pilot with a console window (for debugging). Windows only.
- `-lw` or `--log-wnd` – Open G-Pilot with log browser window.

## Examples

```text
G-Pilot.exe --log-to-file --trim-log --config-type json
G-Pilot.exe --console
```

You can combine options as needed. For more details see `main.cpp` in the source code.
