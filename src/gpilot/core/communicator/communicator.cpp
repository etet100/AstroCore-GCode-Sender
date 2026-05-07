#include "communicator.h"
#include "commandbuffer.h"
#include "overrides.h"
#include <QVector3D>
#include <QDebug>
#include <QCheckBox>
#include <QMessageBox>
#include <QTextCursor>
#include <QRegularExpression>
#include "core/state_behavior/initializationbehavior.h"
#include "core/state_behavior/reconnectingbehavior.h"
#include "core/state_behavior/homingbehavior.h"
#include "core/machine/modalstateparser.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"

Communicator::Communicator(
    QObject *parent,
    AbstractConnection *connection,
    Configuration *configuration
) : QObject(parent),
    m_connection(connection),
    m_configuration(configuration),
    m_sbManager(this),
    m_queryMachineStateTimer(nullptr),
    m_machineStateDictionary({
        {MachineState::Unknown, "Unknown"},
        {MachineState::Idle, "Idle"},
        {MachineState::Alarm, "Alarm"},
        {MachineState::Run, "Run"},
        {MachineState::Home, "Home"},
        {MachineState::Hold0, "Hold:0"},
        {MachineState::Hold1, "Hold:1"},
        {MachineState::Queue, "Queue"},
        {MachineState::Check, "Check"},
        {MachineState::Door0, "Door:0"},
        {MachineState::Door1, "Door:1"},
        {MachineState::Door2, "Door:2"},
        {MachineState::Door3, "Door:3"},
        {MachineState::Jog, "Jog"},
        {MachineState::Sleep, "Sleep"}
    })
{
    m_reseting = false;
    m_resetCompleted = true;
    // m_aborting = false;
    m_statusReceived = false;
    m_spindleCW = true;
    m_comApi = new CommunicatorApi(this);
    m_overrides = new Overrides(this);
    m_posTracker = new PositionTracker(this);
    connect(m_posTracker, &PositionTracker::machinePosChanged, this, &Communicator::machinePosChanged);
    connect(m_posTracker, &PositionTracker::toolPositionReceived, this, &Communicator::toolPositionReceived);

    m_commandScanner = new CommandScanner(this);
    // Re-query work offsets ($#) and modal state ($G) after any command that may have changed them.
    // Triggered on AfterResponse so we only refresh once the controller has actually applied the change.
    connect(m_commandScanner, &CommandScanner::workOffsetCommandDetected, this, [this](CommandScanner::Stage stage) {
        if (stage != CommandScanner::Stage::AfterResponse) {
            return;
        }
        sendCommand(CommandSource::Communicator, "$#", TABLE_INDEX_UTIL1, true);
        sendCommand(CommandSource::Communicator, "$G", TABLE_INDEX_UTIL1, true);
    }, Qt::QueuedConnection);

    // CommandBuffer: the two-level GRBL command queue.
    m_commandBuffer = new CommandBuffer(connection);

    // Callback: behavior notification when a command gets a response.
    m_commandBuffer->setResponseHandler([this](
        const QString& command,
        CommandAttributes& attrs,
        const CmdStatus& status,
        const QString& data,
        const QStringList& lines
    ) -> bool {
        // Store device configuration and state from query responses
        if (command == "$$" && status.ok) {
            processDeviceConfiguration(lines);
        }

        if (command == "$G" && status.ok) {
            processGCodeParserState(attrs, lines[0]);
        }

        if (command == "$#" && status.ok) {
            processOffsetsVars(lines);
        }

        if (status.ok) {
            m_commandScanner->scan(command, CommandScanner::Stage::AfterResponse);
        }

        if (attrs.source == CommandSource::Communicator) {
            // Don't send responses to behaviors for commands they originated.
            return true;
        }

        if (!m_sbManager.hasCurrent()) {
            return false;
        }
        AbstractStateBehavior::Result result = m_sbManager.current()->onCommandResponse(
            command, attrs, status, data, lines
        );

        return result != AbstractStateBehavior::Result::ReturnCommandToQueue;
    });

    // Callback: drain queue through the normal sendCommand path
    // so M2/M30 detection and other special cases still apply.
    m_commandBuffer->setQueuedCommandSender([this](
        CommandSource source,
        const QString& commandLine,
        int tableIndex,
        CommandCallback callback
    ) -> SendCommandResult {
        return sendCommand(source, commandLine, tableIndex, false, callback);
    });

    // Forward CommandBuffer signals to Communicator signals.
    connect(m_commandBuffer, &CommandBuffer::commandSent,
            this, &Communicator::commandSent);

    connect(m_commandBuffer, &CommandBuffer::commandCompleted,
            this, &Communicator::onCommandBufferCompleted);

    m_sbManager.execute(new InitializationBehavior(), false, m_comApi);

    resetStateVariables();

    // this->connect(m_connection, &AbstractConnection::error, this, &Communicator::onConnectionError(QString));
    if (m_connection) {
        connect(m_connection, &AbstractConnection::lineReceived, this, &Communicator::onConnectionLineReceived, Qt::QueuedConnection);
    }

    connect(&m_stateBehaviorTransitionTimer, &QTimer::timeout, this, &Communicator::processStateBehaviorTransition);
    m_stateBehaviorTransitionTimer.start(1000);

}

Communicator::~Communicator()
{
    stopQueryingMachineState();
}

void Communicator::deinit()
{
    m_stateBehaviorTransitionTimer.stop();
    stopQueryingMachineState();
    clearCommandsAndQueue();
}

void Communicator::resetStateVariables()
{
    m_deviceContext.reset();
    if (m_posTracker) m_posTracker->reset();
}

// Called by CommandBuffer after a command response is fully processed.
void Communicator::onCommandBufferCompleted(
    CommandAttributes attributes,
    CmdStatus status,
    QStringList lines)
{
    // Handle special command responses that need processing here.
    const QString command = GcodePreprocessorUtils::removeComment(attributes.commandLine).toUpper();

    if (command == "$#" && status.ok) {
        m_posTracker->processOffsetsVars(lines);
    }

    emit commandResponseReceived(attributes);
    emit responseReceived(attributes.commandLine, attributes.tableIndex, attributes.response);

    // Notify the active behavior even for responses that were not routed through
    // onCommandResponse (source == Communicator). Lets behaviors react to buffer
    // drain unconditionally — needed e.g. by RunningBehavior to finish a deferred
    // transition to Idle after the trailing $#/$G responses.
    if (m_sbManager.hasCurrent()) {
        m_sbManager.current()->onResponseProcessed();
    }
}

/**
 * @param tableIndex -1 - ui commands, -2 - utility commands, -3 - utility commands
 */
SendCommandResult Communicator::sendCommand(
    CommandSource source,
    QString commandLine,
    int tableIndex,
    bool wait,
    CommandCallback callback
) {
    // Handle special console commands that should not go to the machine.
    if (source == CommandSource::Console) {
        QString trimmed = GcodePreprocessorUtils::removeComment(commandLine);
        if (trimmed == "$H") {
            m_sbManager.current()->action(Action::Home);
            return SendCommandResult::Status::Done;
        }
    }

    if (!m_connection->isConnected() || !m_resetCompleted) {
        return SendCommandResult::Status::Done;
    }

    if (commandLine.isEmpty()) {
        return SendCommandResult::Status::Empty;
    }

    commandLine = commandLine.toUpper();

    // Detect M2/M30/M6/M25 end-of-program commands to update sender state.
    // const QString command = GcodePreprocessorUtils::removeComment(commandLine);
    // static QRegularExpression M230("(M0*2|M30|M0*6|M25)(?!\\d)");
    // static QRegularExpression M6("(M0*6)(?!\\d)");
    // if ((m_senderState == SenderState::Transferring) && command.contains(M230)) {
    //     if (
    //         !command.contains(M6) ||
    //         m_configuration->senderModule().useToolChangeCommands() ||
    //         m_configuration->senderModule().pauseSenderOnToolChange()
    //     ) {
    //         setSenderStateAndEmitSignal(SenderState::Pausing);
    //     }
    // }

    m_commandScanner->scan(commandLine, CommandScanner::Stage::BeforeSend);

    return m_commandBuffer->enqueue(source, commandLine, tableIndex, wait, callback);
}

void Communicator::sendRealtimeCommand(QString command)
{
    if (!m_connection->isConnected() || !m_resetCompleted) return;
    m_commandBuffer->sendRealtime(command);
}

void Communicator::sendRealtimeCommand(int command)
{
    m_commandBuffer->sendRealtime(command);
}

void Communicator::queryMachineState()
{
    m_connection->sendByteArray(QByteArray(1, '?'));
}

void Communicator::queryMachineConfiguration()
{
    sendCommand(CommandSource::System, "$$");
}

// Process new state requested by the current state behavior.
void Communicator::processStateBehaviorTransition()
{
    if (m_sbManager.hasPendingResume()) {
        assert(m_sbManager.current() != nullptr);
        m_sbManager.clearPendingResume();
        m_sbManager.resumePrevious(m_comApi);

        return;
    }

    if (m_sbManager.hasPendingTransition()) {
        assert(m_sbManager.current() != nullptr);
        // Clear to avoid re-entrance.
        AbstractStateBehavior *nsb = m_sbManager.next();
        AbstractStateBehavior::TransitionKind kind = m_sbManager.pendingTransitionKind();
        m_sbManager.requestTransition(nullptr);
        m_sbManager.execute(nsb, true, m_comApi, kind);
    }
}

void Communicator::sendCommands(CommandSource source, QString commands, int tableIndex)
{
    sendCommands(source, commands.split("\n"), tableIndex);
}

void Communicator::sendCommands(CommandSource source, QStringList commands, int tableIndex)
{
    bool waitFlag = false;
    foreach (QString cmd, commands) {
        SendCommandResult r = sendCommand(source, cmd.trimmed(), tableIndex, waitFlag);
        if (r == SendCommandResult::Status::Done || r == SendCommandResult::Status::Queue) waitFlag = true;
    }
}

void Communicator::clearCommandsAndQueue()
{
    m_commandBuffer->clear();
}

void Communicator::clearQueue()
{
    m_commandBuffer->clearQueue();
}

// void Communicator::reset()
// {
//     assert(m_sbManager.current() != nullptr);
//     m_sbManager.current()->action(Action::Reset);
// }

// void Communicator::unlock()
// {
//     assert(m_sbManager.current() != nullptr);
//     m_sbManager.current()->action(Action::Unlock);
// }

// void Communicator::abort()
// {
//     // @TODO is CommandSource::Program correct here??
//     if (isSenderState(SenderState::Paused, SenderState::ChangingTool)) {
//         sendCommand(CommandSource::GeneralUI, "M2", TABLE_INDEX_UI, false);
//     } else {
//         sendCommand(CommandSource::GeneralUI, "M2", TABLE_INDEX_UI, true);
//     }
// }

bool Communicator::setConnection(AbstractConnection *newConnection, bool force)
{
    if (!force && m_connection != nullptr) {
        return false;
    }

    m_connection = newConnection;

    if (!m_connection) {
        return true;
    }

    connect(m_connection, &AbstractConnection::lineReceived, this, &Communicator::onConnectionLineReceived);
    connect(m_connection, &AbstractConnection::stateChanged, this, &Communicator::onConnectionStateChanged);

    m_commandBuffer->setConnection(newConnection);
    emit connectionChanged(m_connection);

    return true;
}

bool Communicator::startReconnecting(AbstractConnection *connection)
{
    return execute(new ReconnectingBehavior(connection));
}

AbstractConnection *Communicator::connection()
{
    return m_connection;
}

bool Communicator::isMachineConfigurationReady() const
{
    return m_deviceContext.hasPhysicalConfig();
}

// void Communicator::probe()
// {
//     m_sbManager.current()->action(Action::Probe);
// }

void Communicator::resetGRBLConfiguration()
{
    sendCommand(CommandSource::GeneralUI, "$RST=$", TABLE_INDEX_UI);
}

// void Communicator::home()
// {
//     m_sbManager.current()->action(Action::Home);
// }

bool Communicator::execute(AbstractStateBehavior *sb, bool force)
{
    return m_sbManager.execute(sb, force, m_comApi);
}

void Communicator::storeParserState()
{
    m_storedParserState = m_lastParserState.remove(QRegularExpression("GC:|\\[|\\]|G[01234]\\s|M[0345]+\\s|\\sF[\\d\\.]+|\\sS[\\d\\.]+"));
}

void Communicator::restoreParserState()
{
    if (!m_storedParserState.isEmpty()) {
        sendCommand(CommandSource::System, m_storedParserState, TABLE_INDEX_UI);
    }
}

void Communicator::completeTransfer()
{
    m_storedParserState.clear();

    if (m_configuration->senderModule().useProgramEndCommands())
        sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration->senderModule().programEndCommands());

    emit transferCompleted();
}

void Communicator::onConnectionError(QString message)
{
    qDebug() << "[Communicator] AbstractConnection error: " << message;
}

void Communicator::onConnectionStateChanged(ConnectionState state)
{
    qDebug() << "[Communicator] AbstractConnection state changed to " << static_cast<int>(state);

    if (state == ConnectionState::Connected) {
        m_lastAlarmCode = 0;
    }

    m_sbManager.current()->onConnectionStateChanged(state);

    emit connectionStateChanged(state);
}

void Communicator::onStateRequestsTransition(AbstractStateBehavior *sb, AbstractStateBehavior *nsb,
                                              AbstractStateBehavior::TransitionKind kind)
{
    qDebug() << "[Communicator] State transition requested from " << sb->description()
             << " to " << nsb->description()
             << (kind == AbstractStateBehavior::TransitionKind::Suspend ? "(suspend)" : "(replace)");
    m_sbManager.requestTransition(nsb, kind);
}

void Communicator::onStateRequestsResume()
{
    qDebug() << "[Communicator] State resume requested";
    m_sbManager.requestResume();
}

void Communicator::respondToPrompt(const QString &promptId, const QString &choiceId)
{
    AbstractStateBehavior *sb = m_sbManager.current();
    if (!sb) {
        qWarning() << "[Communicator] respondToPrompt with no active behavior";

        return;
    }
    sb->respondToPrompt(promptId, choiceId);
    // The behavior may have requested resumePrevious() — drive the state
    // machine immediately so the UI doesn't wait for the 1s fallback timer.
    processStateBehaviorTransition();
}

void Communicator::onStateError(AbstractStateBehavior *sb, QString message)
{
    qDebug() << "[Communicator] State error: " << message;
}

void Communicator::startQueryingMachineState()
{
    if (m_queryMachineStateTimer != nullptr) {
        return;
    }

    m_queryMachineStateTimer = new QTimer(this);
    connect(m_queryMachineStateTimer, &QTimer::timeout, this, [this]() {
        queryMachineState();
    });

    m_queryMachineStateTimer->start(m_configuration->connectionModule().queryStateInterval());
}

void Communicator::stopQueryingMachineState()
{
    if (m_queryMachineStateTimer) {
        qDebug() << "[Communicator] Stopping machine state querying timer";
        m_queryMachineStateTimer->stop();
        m_queryMachineStateTimer->deleteLater();
        m_queryMachineStateTimer = nullptr;
    }
}
