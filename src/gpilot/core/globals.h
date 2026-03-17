#ifndef GLOBALS_H
#define GLOBALS_H

// Do not include anything else here, only standard definitions and Qt includes

#include <QObject>
#include <QDebug>
#include <QVector3D>
#include <QString>

// GRBL live commands
#define GRBL_LIVE_SOFT_RESET 0x18
#define GRBL_LIVE_STATUS_REPORT '?'
#define GRBL_LIVE_CYCLE_START '~'
#define GRBL_LIVE_FEED_HOLD '!'

#define GRBL_LIVE_RAPID_FULL_RATE 0x95
#define GRBL_LIVE_RAPID_HALF_RATE 0x96
#define GRBL_LIVE_RAPID_QUARTER_RATE 0x97

#define GRBL_LIVE_FEED_FULL_RATE 0x90
#define GRBL_LIVE_FEED_INCREASE_10 0x91
#define GRBL_LIVE_FEED_DECREASE_10 0x92
#define GRBL_LIVE_FEED_INCREASE_1 0x93
#define GRBL_LIVE_FEED_DECREASE_1 0x94

#define GRBL_LIVE_SPINDLE_FULL_SPEED 0x99
#define GRBL_LIVE_SPINDLE_INCREASE_10 0x9A
#define GRBL_LIVE_SPINDLE_DECREASE_10 0x9B
#define GRBL_LIVE_SPINDLE_INCREASE_1 0x9C
#define GRBL_LIVE_SPINDLE_DECREASE_1 0x9D

#define GRBL_LIVE_JOG_CANCEL 0x85

// GRBL errors
#define GRBL_ERROR_EXPECTED_COMMAND_LETTER        1
#define GRBL_ERROR_BAD_NUMBER_FORMAT              2
#define GRBL_ERROR_INVALID_STATEMENT              3
#define GRBL_ERROR_VALUE_LESS_THAN_ZERO           4
#define GRBL_ERROR_HOMING_DISABLED                5
#define GRBL_ERROR_EEPROM_READ_FAIL               7
#define GRBL_ERROR_NOT_IDLE                       8
#define GRBL_ERROR_GCODE_LOCK                     9
#define GRBL_ERROR_HOMING_NOT_ENABLED             10
#define GRBL_ERROR_LINE_OVERFLOW                  11
#define GRBL_ERROR_LINE_LENGTH_EXCEEDED           14
#define GRBL_ERROR_TRAVEL_EXCEEDED                15
#define GRBL_ERROR_SETTING_DISABLED               17
#define GRBL_ERROR_UNSUPPORTED_COMMAND            20
#define GRBL_ERROR_MODAL_GROUP_VIOLATION          21
#define GRBL_ERROR_UNDEFINED_FEED_RATE            22

enum class GrblError {
    ExpectedCommandLetter = 1,
    BadNumberFormat = 2,
    InvalidStatement = 3,
    ValueLessThanZero = 4,
    HomingDisabled = 5,
    EEPROMReadFail = 7,
    NotIdle = 8,
    GcodeLock = 9,
    HomingNotEnabled = 10,
    LineOverflow = 11,
    LineLengthExceeded = 14,
    TravelExceeded = 15,
    SettingDisabled = 17,
    UnsupportedCommand = 20,
    ModalGroupViolation = 21,
    UndefinedFeedRate = 22
};

// GRBL alarms
#define GRBL_ALARM_HARD_LIMITS          1
#define GRBL_ALARM_SOFT_LIMITS          2
#define GRBL_ALARM_RESET                3
#define GRBL_ALARM_PROBE_FAIL_1         4
#define GRBL_ALARM_PROBE_FAIL_2         5
#define GRBL_ALARM_HOMING_FAIL_1        6
#define GRBL_ALARM_HOMING_FAIL_2        7
#define GRBL_ALARM_HOMING_FAIL_3        8
#define GRBL_ALARM_HOMING_FAIL_4        9
#define UCNC_ALARM_FAILED_AUTOLEVEL     10
#define UCNC_ALARM_LIMITS_ACTIVE        11
#define UCNC_ALARM_TOOL_SYNC_FAIL       12
#define UCNC_ALARM_LIMITS_TRIPPED       13
#define FLUIDNC_ALARM_SPINDLE_CONTROL         10
#define FLUIDNC_ALARM_STARTUP_PIN             11
#define FLUIDNC_ALARM_HOMING_AMBIGUOUS_SWITCH 12
#define FLUIDNC_ALARM_HARD_STOP               13
#define FLUIDNC_ALARM_UNHOMED                 14
#define FLUIDNC_ALARM_INIT                    15
#define FLUIDNC_ALARM_EXPANDER_RESET          16
#define FLUIDNC_ALARM_GCODE_ERROR             17
#define FLUIDNC_ALARM_PROBE_HARD_LIMIT        18

// enum class ExecAlarm : uint8_t {
//     HardLimit             = 1,
//     SoftLimit             = 2,
//     AbortCycle            = 3,
//     ProbeFailInitial      = 4,
//     ProbeFailContact      = 5,
//     HomingFailReset       = 6,
//     HomingFailDoor        = 7,
//     HomingFailPulloff     = 8,
//     HomingFailApproach    = 9,
//     SpindleControl        = 10,
//     StartupPin            = 11,  // control or limit input pin active
//     HomingAmbiguousSwitch = 12,
//     HardStop              = 13,
//     Unhomed               = 14,
//     Init                  = 15,
//     ExpanderReset         = 16,
//     GCodeError            = 17,
//     ProbeHardLimit        = 18,
// };


enum class GrblAlarm {
    HardLimits = 1,
    SoftLimits = 2,
    Reset = 3,
    ProbeFail1 = 4,
    ProbeFail2 = 5,
    HomingFail1 = 6,
    HomingFail2 = 7,
    HomingFail3 = 8,
    HomingFail4 = 9
};

#define CONFIGURATION_FILE "settings_.ini"

// tableIndex:
// 0...n - commands from g-code program
// -1 - ui commands
// -2 - utility commands
// -3 - utility commands
#define TABLE_INDEX_MIN_GCODE 0
#define TABLE_INDEX_UI -1
#define TABLE_INDEX_UTIL1 -2
#define TABLE_INDEX_UTIL2 -3

enum class Units {
    Inches,
    Millimeters
};

enum GRBLCommand {
    Reset,
    Home,
    Unlock,
    JogStop,
    Probe,
    ZeroZ,
    ZeroXY,
};

enum class HomingDir {
    Negative = 0,
    Positive
};

class HomingDirs {
    public:
        HomingDirs() {
            m_x = HomingDir::Positive;
            m_y = HomingDir::Negative;
            m_z = HomingDir::Negative;
        }

        HomingDirs(HomingDir x, HomingDir y, HomingDir z) {
            this->m_x = x;
            this->m_y = y;
            this->m_z = z;
        }

        HomingDir x() { return m_x; }
        HomingDir y() { return m_y; }
        HomingDir z() { return m_z; }

    private:
        HomingDir m_x;
        HomingDir m_y;
        HomingDir m_z;
};

struct CmdStatus {
    bool ok;
    int errorCode;
};

enum class Axis : int {
    Probe = -2, // not a real axis, used for testing probe position
    None = -1,
    X,
    Y,
    Z,
    A,
    B,
    C,
};

enum class JoggindDir {
    None,
    XPlus,
    XMinus,
    YPlus,
    YMinus,
    XMinusYMinus,
    XMinusYPlus,
    XPlusYPlus,
    XPlusYMinus,
    ZPlus,
    ZMinus
};

typedef QVector3D JoggingVector;

enum class SenderState {
    Unknown = -1,
    Transferring = 0,
    Pausing = 1,
    Paused = 2,
    Stopping = 3,
    Stopped = 4,
    ChangingTool = 5,
    Pausing2 = 6
};

enum class MachineState : int {
    Unknown = 0,
    Idle = 1,
    Alarm = 2,
    Run = 3,
    Home = 4,
    Hold0 = 5,
    Hold1 = 6,
    Queue = 7,
    Check = 8,
    Door0 = 9,
    Door1 = 10,
    Door2 = 11,
    Door3 = 12,
    Jog = 13,
    Sleep = 14
};

enum class SendCommandResult : int {
    Done = 0,
    Empty = 1,
    Queue = 2
};

enum class CommandSource : uint8_t {
    Console,
    GeneralUI,
    Program,
    // Commands like "send before/after pause" confugured by user
    ProgramAdditionalCommands,
    System,
};

typedef std::function<void(void *)> CommandCallback;

struct CommandQueue {
    QString commandLine;
    int tableIndex;
    CommandSource source;
    CommandCallback callback;

    CommandQueue() {
    }

    CommandQueue(CommandSource source, QString commandLine, int tableIndex, CommandCallback callback = nullptr) {
        this->commandLine = commandLine;
        this->tableIndex = tableIndex;
        this->source = source;
        this->callback = callback;
    }
};

struct CommandAttributes : CommandQueue {
    int length;
    int commandIndex; // used for console
    QString command;
    QString response = "";

    CommandAttributes() : CommandQueue() {
    }

    CommandAttributes(const CommandAttributes& other) : CommandQueue() {
        CommandAttributes(other.source, other.commandIndex, other.tableIndex, other.commandLine);
        source = other.source;
        length = other.length;
        commandIndex = other.commandIndex;
        tableIndex = other.tableIndex;
        commandLine = other.commandLine;
        response = other.response;
        callback = other.callback;
    }

    CommandAttributes(CommandSource source, int commandIndex, int tableIndex, QString commandLine, CommandCallback callback = nullptr)
        : CommandQueue(source, commandLine, tableIndex, callback)
    {
        this->length = commandLine.length() + 1;
        this->commandIndex = commandIndex;
    }

    CommandAttributes(CommandSource source, QString commandLine, CommandCallback callback = nullptr)
        : CommandAttributes(source, -1, TABLE_INDEX_UI, commandLine, callback)
    {
    }
};

enum class ConnectionState
{
    NoConnectionDevice,
    Initialization,
    Connecting,
    Connected,
    Disconnecting,
    Disconnected,
    InvalidConfiguration,
};

#endif // GLOBALS_H

