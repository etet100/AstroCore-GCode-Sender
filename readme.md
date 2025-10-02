Status: DOUBTS
--------

I started this project because Candle had not been developed for a long time and contained many bugs. Unfortunately, my perfectionism made me start rewriting it from scratch. At this point, I find myself at a crossroads and do not know whether to continue this project. If someone wanted to join the work on it, I would be very happy.

At this point, it makes no sense to compile the code because it is being rebuilt and does not work properly.

G-Pilot (formerly Candle)
-----------
<img src="screenshots/gpilot_big.jpg" width="200" height="200" align="right">
What G stands for?

- G-code Pilot: G-code is a programming language used to control CNC machines, so "G-Pilot" may suggest that the program is used for piloting or controlling using G-code.
- Guided Pilot: "G-Pilot" may also suggest that the program provides guidance or leads the user through processes related to CNC machining, similar to how a pilot guides an airplane.
- Global Pilot: This may suggest that the program offers solutions on a global scale, able to handle various types of CNC machines and be a versatile tool for controlling them.
- Genius Pilot: "G-Pilot" may suggest that the program is smart or advanced, similar to a pilot with high piloting skills.
- Graphical Pilot: If the program offers a graphical interface for controlling CNC machines, the name "G-Pilot" may suggest that it is a graphics-based tool.

*This fork is based on the Candle `experimental` branch. The main goal is to add joystick/joypad support. Other than that, I'm making improvements/bugfixes at my discretion.*

*Any help is welcome!*

What is G-Pilot?
-----------

GRBL/uCNC controller application with G-Code visualizer written in Qt.

Supported functions:
* Controlling GRBL-based cnc-machine via console commands, buttons on form, numpad.
* Monitoring cnc-machine state.
* Loading, editing, saving and sending of G-code files to cnc-machine.
* Visualizing G-code files.
* Camera
* Joystick/Joypad/Controller support.
* Customizable interface.
* uCNC/grblHAL virtual modes (cnc machine simulator).

System requirements for running "G-Pilot":
-------------------
* Windows 10/Linux x86(not tested att all!)
* Graphics card with OpenGL 3.0 support
* 150 MB free storage space

Usefull links:

* https://github.com/Paciente8159/uCNC (modern firmware, inspired by Grbl and LinuxCNC)
* https://github.com/grbl/grbl (original GRBL firmware)
* https://github.com/grblHAL (modular, mostly compatible with GRBL)

Build requirements:
-------------------
Qt 6.8 with MinGW/GCC 64bit compiler

Start with:

git clone --recurse-submodules https://github.com/etet100/G-Pilot-Formerly-Candle

Connection modes:
-----------------

G-Pilot supports the following connection modes:
* Serial port
* Raw TCP, uses exactly the same protocol as serial port mode, no additional handshaking is performed
* uCNC virtual mode, no real hardware needed
* grblHAL virtual mode, no real hardware needed

![screenshot](/screenshots/screenshot_connection_modes.png)

Architecture:
-------------

The original Candle was built in a way that was not transparent and difficult to modify. My version will be built modularly, in a way that is tentatively presented in the diagram below. One of the changes is the complete separation of the interface from the machine control logic. This has the following advantages:

* General transparency and manageability of the code
* The ability to quickly change the UI, even with the possibility of creating it in a * different technology
* Easier handling of new communication methods
* Much easier addition of new features
* Easier testing

![screenshot](/screenshots/arch1.png)

Application states:
-------------------

The diagram below shows the possible states of the application. Transitions between states are triggered by events such as changing the state of the machine, clicking buttons, etc.

```mermaid
stateDiagram-v2
%% Nodes
    Idle
    Running
    Pausing
    Paused
    Resuming
    Stopping
    Stopped
    Running
    Run
    Halted
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

%% Edge connections between nodes
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

Configurations:
---------------

Another module begging for a rewrite is the configuration storage mechanism. Of course, it will be detached into a separate module. Settings sets will be wrapped in separate classes. The reading and writing of fields will be partially automated. Persistence layer will be separated from the configuration logic. The configuration will be stored in a local file or in a cloud, for example Dropbox or Google Drive.

![screenshot](/screenshots/arch2.png)

Downloads:
----------
Only Windows preview version is available at the moment. They are not stable and contain many critical bugs. The Linux version will be available sooner or later.

How it looks:
-------------

Main window:

![main](/screenshots/screenshot_main_light.png)

Dark mode:

![main](/screenshots/screenshot_main_dark.png)

Settings:

![settings](/screenshots/screenshot_settings.png)

GRBL configurator:

![grbl configurator](/screenshots/screenshot_grbl_configurator.png)
