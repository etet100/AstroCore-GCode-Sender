---
title: Application States
---

# Application States

AstroCore uses a clear state machine to manage application and machine states. Transitions between states are triggered by events such as machine state changes or user actions.

## State diagram

```mermaid
stateDiagram-v2
    Idle
    Running
    Pausing
    Paused
    Resuming
    Stopping
    Stopped
    Jogging
    JoggingWaitingForIdle
    Halted
    Error
    SoftReset
    ChangingTool
    [AnyState]

    state Pause {
        Pausing --> Paused
    }

    state Jog {
        Jogging --> JoggingWaitingForIdle
        JoggingWaitingForIdle --> Jogging
    }

    [*] --> Connecting
    Connecting --> Connected
    Connecting --> ConnectingError
    ConnectingError --> Connecting : wait 3s
    Connected --> Idle : if SoftReset enabled
    Connected --> SoftReset : if SoftReset disabled
    SoftReset --> Idle
    Idle --> Run : run clicked
    Run --> Running
    Running --> Pausing : use pause or g-code
    Running --> Stopping
    Running --> Halted
    Running --> ChangingTool
    ChangingTool --> Stopping
    ChangingTool --> Running
    Paused --> Resuming : "on resume" g-code
    Paused --> Running
    Paused --> Stopping : stop clicked
    Halted --> Running
    Halted --> Stopping
    Idle --> Homing : home clicked
    Homing --> Idle
    Resuming --> Running
    Stopping --> Stopped
    Stopped --> Idle
    Idle --> Jog
    Jog --> Idle

    [AnyState] --> Error: If error
    Error --> [*] : Error

    [AnyState] --> Disconnected
    Disconnected --> [*] : Connecting
```