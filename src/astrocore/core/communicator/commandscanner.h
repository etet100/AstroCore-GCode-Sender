#pragma once

#include <QObject>

// Scans G-code commands and emits signals for significant patterns.
// Called from Communicator at two stages: before the command is sent and
// after a successful response is received.
class CommandScanner : public QObject
{
    Q_OBJECT

public:
    enum class CommandType {
        None,
        WorkOffset,  // G10, G92, G54-G59, $RST=#
        Homing,      // $H, G28, G30
        Pause,       // M0, M1, M25
        ToolChange,  // M6
    };

    enum class Stage {
        BeforeSend,
        AfterResponse,
    };
    Q_ENUM(Stage)

    explicit CommandScanner(QObject* parent = nullptr);

    // Classify a command line. Returns the detected command type, or None.
    // Strips comments internally. Does NOT emit signals.
    static CommandType classify(const QString& commandLine);

    // Classify and emit a signal for the detected type, tagged with the stage.
    void scan(const QString& commandLine, Stage stage);

signals:
    void workOffsetCommandDetected(CommandScanner::Stage stage);
    void homingCommandDetected(CommandScanner::Stage stage);
    void pauseCommandDetected(CommandScanner::Stage stage);
    void toolChangeCommandDetected(CommandScanner::Stage stage);
};
