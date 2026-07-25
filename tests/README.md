# AstroCore unit tests

Unit tests for the pure-logic parts of the core layer, built with **Qt Test**
and qmake. Each suite compiles only the source files it needs directly into a
small standalone test executable — no full application build required.

## Layout

```
tests/
  tests.pro          subdirs project (build everything)
  common.pri         shared config (C++20, testlib, INCLUDEPATH to src/astrocore)
  gcode/             GCode facade (cursor, responses, overlays, coalesced signals)
  gcodeprogram/      GCodeProgram — pure data model (no QObject/signals)
  gcodecursor/       GCodeCursor — execution cursor over a program
  gcodepreprocessorutils/  static G-Code string + arc geometry helpers
  grblparsers/       StatusReportProcessor, ProbeResponseParser, ModalStateParser
  config/            config modules (derived logic) + JSON persister/provider round-trip
  persistence/       INI/JSON/XML providers read injected fixtures; INI/XML write→read round-trips
```

## Build & run

Three ways, in order of convenience:

### 1. Runner script (recommended for a full run)

```powershell
pwsh tests/run.ps1                 # build + run every suite, print a summary
pwsh tests/run.ps1 -Filter gcode*  # only matching suites
pwsh tests/run.ps1 -NoBuild        # re-run without rebuilding
```

It puts the Qt kit on `PATH`, builds out-of-source into `build/tests/`, runs
each `tst_*.exe`, and prints one line per suite (green/red) plus any failures.
Exit code is non-zero if anything fails, so it works in CI. Override the kit
with `-QtKit` / `-Toolchain` or the `QT_KIT` / `QT_TOOLCHAIN` env vars.

### 2. Qt Creator (best for day-to-day)

Open `tests/tests.pro` as a project (or add it as a subproject). Because each
suite sets `CONFIG += testcase`, the suites show up in the **Test Results**
pane — run one test, one suite, or all, and click failures to jump to the line.

### 3. Command line by hand

Use the same Qt 6.11 kit as the app (llvm-mingw), with its `bin` dirs on `PATH`:

```sh
qmake C:/Projekty/Qt/GPilot/tests/tests.pro
mingw32-make -j4
mingw32-make check                 # runs every suite's test target
# or run one exe directly (add -o results.txt,txt to capture a report):
gcode/release/tst_gcode.exe
```

Note: in some shells QtTest's stdout is swallowed — use `-o out.txt,txt` (the
runner does this) or `mingw32-make check`, and rely on the exit code.

## Adding a suite

1. Create `tests/<name>/<name>.pro`, `include(../common.pri)`, set `TARGET`,
   list the `$$ASTROCORE_SRC/...` sources to compile in and your `tst_<name>.cpp`.
2. Add `<name>` to `SUBDIRS` in `tests.pro`.
3. Pick source files with few dependencies (no UI/OpenGL/hardware/singletons).
   See the testability survey for good candidates.

## Tests that pin down known quirks

Some tests assert surprising-but-shipping behavior on purpose, so a refactor
cannot change it silently. They say so in a comment ("Known quirk/limitation").
Current ones live in `gcodepreprocessorutils/`: nested `(...)` comments are only
stripped up to the first `)`, a minus sign directly after a digit is dropped by
the tokenizer, `parseGCodes` loses decimal subtypes (`G38.2` → `38`), and arc
degree mode truncates the segment count while millimetre mode rounds up. If you
intend to change any of these, update the test and its comment together.

## Note on GCode signals

`GCode` coalesces view-update notifications behind a 100 ms timer. Tests call
`setAutoFlushEnabled(false)` and drive delivery with `flushPendingUpdates()`
so they stay fast and deterministic instead of waiting on the timer.
