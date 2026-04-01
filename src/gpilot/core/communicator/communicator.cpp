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
    Connection *connection,
    Configuration *configuration
) : QObject(parent),
    m_connection(connection),
    m_configuration(configuration),
    m_jogger(*this, configuration->joggingModule()),
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
    m_aborting = false;
    m_statusReceived = false;
    m_spindleCW = true;
    m_comApi = new CommunicatorApi(this);
    m_overrides = new Overrides(this);
    m_posTracker = new PositionTracker(this);
    connect(m_posTracker, &PositionTracker::machinePosChanged, this, &Communicator::machinePosChanged);
    connect(m_posTracker, &PositionTracker::toolPositionReceived, this, &Communicator::toolPositionReceived);

    m_commandScanner = new CommandScanner(this);
    // Re-query work offsets ($#) after any command that may have changed them.
    // QueuedConnection avoids re-entering sendCommand() mid-call.
    connect(m_commandScanner, &CommandScanner::workOffsetCommandDetected, this, [this]() {
        sendCommand(CommandSource::Communicator, "$#", TABLE_INDEX_UTIL1, true);
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
        // Store current coordinate system
        if (command == "$G" && status.ok) {
            auto modal = ModalStateParser::parse(lines[0]);
            if (modal) {
                qDebug() << "[Communicator] Detected coordinate system: " << modal->coordinateSystem;
            }
        }

        if (attrs.source == CommandSource::Communicator) {
            // Don't send responses to behaviors for commands they originated.
            return true;
        }

        if (!m_sbManager.hasCurrent()) {
            return false;
        }
        StateBehavior::Result result = m_sbManager.current()->onCommandResponse(
            command, attrs, status, data, lines
        );

        return result != StateBehavior::Result::ReturnCommandToQueue;
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

    // this->connect(m_connection, &Connection::error, this, &Communicator::onConnectionError(QString));
    if (m_connection) {
        connect(m_connection, &Connection::lineReceived, this, &Communicator::onConnectionLineReceived, Qt::QueuedConnection);
    }

    setSenderStateAndEmitSignal(SenderState::Stopped);
}

Communicator::~Communicator()
{
    stopQueryingMachineState();
}

void Communicator::deinit()
{
    stopQueryingMachineState();
    clearCommandsAndQueue();
}

void Communicator::resetStateVariables()
{
    m_machineState = MachineState::Unknown;
    m_senderState = SenderState::Unknown;
    if (m_posTracker) m_posTracker->reset();
    m_machineConfiguration = nullptr;
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

    m_commandScanner->scan(commandLine);

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
    if (m_sbManager.hasPendingTransition()) {
        assert(m_sbManager.current() != nullptr);
        // Clear to avoid re-entrance.
        StateBehavior *nsb = m_sbManager.next();
        m_sbManager.requestTransition(nullptr);
        m_sbManager.execute(nsb, true, m_comApi);
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

void Communicator::reset()
{
    assert(m_sbManager.current() != nullptr);
    m_sbManager.current()->action(Action::Reset);
}

void Communicator::unlock()
{
    assert(m_sbManager.current() != nullptr);
    m_sbManager.current()->action(Action::Unlock);
}

void Communicator::abort()
{
    // @TODO is CommandSource::Program correct here??
    if (isSenderState(SenderState::Paused, SenderState::ChangingTool)) {
        sendCommand(CommandSource::GeneralUI, "M2", TABLE_INDEX_UI, false);
    } else {
        sendCommand(CommandSource::GeneralUI, "M2", TABLE_INDEX_UI, true);
    }
}

bool Communicator::setConnection(Connection *newConnection, bool force)
{
    if (!force && m_connection != nullptr) {
        return false;
    }

    m_connection = newConnection;

    if (!m_connection) {
        return true;
    }

    connect(m_connection, &Connection::lineReceived, this, &Communicator::onConnectionLineReceived);
    connect(m_connection, &Connection::stateChanged, this, &Communicator::onConnectionStateChanged);

    m_commandBuffer->setConnection(newConnection);
    emit connectionChanged(m_connection);

    return true;
}

bool Communicator::startReconnecting(Connection *connection)
{
    return execute(new ReconnectingBehavior(connection));
}

StateBehavior *Communicator::sb() const
{
    return m_sbManager.current();
}

Connection *Communicator::connection()
{
    return m_connection;
}


void Communicator::setSenderStateAndEmitSignal(SenderState state)
{
    if (m_senderState != state) {
        m_senderState = state;
        emit senderStateChanged(state);
    }

    emit senderStateReceived(state);
}

void Communicator::setMachineStateAndEmitSignal(MachineState state)
{
    if (m_machineState != state) {
        m_machineState = state;
        emit machineStateChanged(state);
    }
}

bool Communicator::isMachineConfigurationReady() const
{
    return m_machineConfiguration != nullptr;
}

bool Communicator::isSenderState(SenderState state) const
{
    return m_senderState == state;
}

void Communicator::probe()
{
    m_sbManager.current()->action(Action::Probe);
}

void Communicator::resetGRBLConfiguration()
{
    sendCommand(CommandSource::GeneralUI, "$RST=$", TABLE_INDEX_UI);
}

void Communicator::home()
{
    m_sbManager.current()->action(Action::Home);
}

bool Communicator::execute(StateBehavior *sb, bool force)
{
    return m_sbManager.execute(sb, force, m_comApi);
}

bool Communicator::finalizeExecute(StateBehavior *sb)
{
    return m_sbManager.finalizeExecute(sb, m_comApi);
}

void Communicator::processConnectionTimer()
{
    processStateBehaviorTransition();
}


double Communicator::toMetric(double value)
{
    return m_machineConfiguration->units() == Units::Millimeters ? value : value * 25.4;
}

double Communicator::toInches(double value)
{
    return m_machineConfiguration->units() == Units::Inches ? value : value / 25.4;
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
    setSenderStateAndEmitSignal(SenderState::Stopped);
    m_streamer->resetProcessed();
    m_storedParserState.clear();

    if (m_configuration->senderModule().useProgramEndCommands())
        sendCommands(CommandSource::ProgramAdditionalCommands, m_configuration->senderModule().programEndCommands());

    emit transferCompleted();
}

void Communicator::onConnectionError(QString message)
{
    qDebug() << "[Communicator] Connection error: " << message;
}

void Communicator::onConnectionStateChanged(ConnectionState state)
{
    qDebug() << "[Communicator] Connection state changed to " << static_cast<int>(state);

    if (state == ConnectionState::Connected) {
        m_lastAlarmCode = 0;
    }

    m_sbManager.current()->onConnectionStateChanged(state);
}

void Communicator::onStateRequestsTransition(StateBehavior *sb, StateBehavior *nsb)
{
    qDebug() << "[Communicator] State transition requested from " << sb->description() << " to " << nsb->description();
    m_sbManager.requestTransition(nsb);
}

void Communicator::onStateError(StateBehavior *sb, QString message)
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
