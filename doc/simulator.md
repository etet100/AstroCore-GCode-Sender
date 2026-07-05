# Simulator Architecture

## Overview

The simulator system emulates CNC firmware (GRBL, FluidNC, uCNC) for testing
without real hardware. Each firmware is compiled as a DLL that communicates
via local sockets.

## Components

### Shared definitions (`io/connection/simulatordefs.h`)

Enums and helpers used by both the main app and the simulator subapp:
- `Simulator::Type` — GRBL, FluidNC, UCNC
- `Simulator::StopFlag` — Running, StopRequested, Stopped
- `Simulator::typeFromString()` / `typeToString()`

### Worker threads (one per simulator type)

Each worker thread loads a specific DLL and calls its entry point:

| Class                        | DLL          | Function   |
|------------------------------|--------------|------------|
| VirtualGRBLWorkerThread      | grblHal.dll  | GRBL()     |
| VirtualFluidNCWorkerThread   | FluidNC.dll  | FluidNC()  |
| VirtualUCNCWorkerThread      | uCNC.dll     | uCNC()     |

All DLL functions have the same signature:
```cpp
void FUNC(QString serverName, QAtomicInt* stopFlag);
```

Platform modes:
- **Dynamic** (Windows): loads DLL at runtime via `QLibrary`
- **Static** (Linux): links against shared library at compile time

Worker thread files are compiled by both the main app and the simulator
subapp — no code duplication.

### VirtualConnection (`io/connection/virtualconnection.h`)

Base class for virtual machine connections in the main app.
Subclasses: `VirtualGRBLConnection`, `VirtualFluidNCConnection`,
`VirtualUCNCConnection`. Each overrides `createWorkerThread()` to
return the matching worker thread.

Two compile-time modes (`VIRTUAL_SIMULATOR_PROCESS` define):

**Thread mode** (default): Creates a worker thread in-process.
The main app creates a `QLocalServer`, the DLL connects back to it.

**Process mode**: Launches `astrocore-simulator.exe` as a separate process.
Same socket protocol — the exe loads the DLL which connects back.

### Simulator subapp (`src/subapps/simulator/`)

Standalone Qt application. Two launch modes:

```
# Launched by main app (process mode):
astrocore-simulator.exe <serverName> <type>

# Standalone (creates own server):
astrocore-simulator.exe --type grbl
```

## Communication Protocol

Two `QLocalSocket` connections from DLL to server:

1. **Data socket** (first connection) — plain text, line-based.
   GCode commands from main app, status responses from DLL.

2. **Control socket** (second connection) — JSON, line-based.
   Commands: `probe_at_current`, `reset_probe`, `set_home`,
   `set_single_limit`, `estop`.

## Flow

```
Main app                              Simulator (thread or process)
────────                              ─────────
1. Create QLocalServer
2. Start worker thread / process
                                      3. Load DLL
                                      4. DLL connects socket 1 (data)
5. Accept → m_socket
                                      6. DLL connects socket 2 (control)
7. Accept → m_controlSocket
8. sendLine("G0 X10") ──────────────> DLL processes command
                       <────────────── DLL sends response
9. sendControlCommand(estop) ───────> DLL handles control cmd
```
