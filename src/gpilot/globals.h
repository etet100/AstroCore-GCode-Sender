#ifndef GLOBALS_H
#define GLOBALS_H

// Do not include anything else here, only standard definitions and Qt includes

#include <QObject>
#include <QDebug>
#include <QVector3D>
#include <QString>

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

enum class JoggindDir {
    None,
    XPlus,
    XMinus,
    YPlus,
    YMinus,
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

enum class DeviceState {
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

enum class SendCommandResult: int {
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
    Initialization,
    Connecting,
    Connected,
    Disconnected,
    InvalidConfiguration,
};

#endif // GLOBALS_H

