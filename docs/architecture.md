---
title: Architecture Overview
---

# Architecture Overview

AstroCore is being refactored into a modular architecture. One of the main goals is to clearly separate the user interface from the machine control logic.

This brings several advantages:

- Better transparency and maintainability of the code
- Ability to change the UI quickly, even in a different technology
- Easier support for new communication methods
- Easier addition of new features
- Easier testing

## High-level diagram

```mermaid
flowchart TD
    PhysicalMachine["Physical machine"]

    Serial["Serial"]
    RawTCP["Raw TCP"]
    Virtual["Virtual"]

    ConnectionManager["Connection manager"]
    Communicator["Communicator"]
    Jogger["Jogger"]
    Streamer["Streamer"]

    Joystick["Joystick Pendant"]
    UI["User interface"]

    Parser["Parser"]
    FileParser["File parser"]
    BufferParser["Buffer parser"]

    PhysicalMachine <--> Serial
    PhysicalMachine <--> RawTCP
    PhysicalMachine <--> Virtual

    Serial --> ConnectionManager
    RawTCP --> ConnectionManager
    Virtual --> ConnectionManager

    ConnectionManager <--> Communicator

    Communicator --> Jogger
    Communicator --> Streamer
    Jogger --> Communicator
    Streamer --> Communicator

    Joystick --> Jogger
    Joystick --> Streamer
    Joystick --> UI

    UI --> Jogger
    UI --> Streamer
    UI --> Parser

    Streamer --> Parser

    Parser --> FileParser
    Parser --> BufferParser
```