#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "core/globals.h"
#include "core/gcode/gcode.h"
#include "core/config/configuration.h"
#include "io/connection/abstractconnection.h"
#include "core/machine/physicalmachineconfiguration.h"
#include "core/machine/devicecontext.h"
#include "positiontracker.h"
#include "core/state_behavior/abstractstatebehavior.h"
#include "statebehaviormanager.h"
#include "machinestatus.h"
#include "overrides.h"
#include "commandbuffer.h"
#include "commandscanner.h"
#include <QTimer>
#include <QPointer>

class Communicator : public QObject
{
    friend class CommunicatorApi;
    friend class StateBehaviorManager;

    Q_OBJECT

    public:
        Communicator(
            QObject *parent,
            AbstractConnection *connection,
            Configuration *configuration
        );
        ~Communicator();
        void deinit();
        SendCommandResult sendCommand(CommandSource source, QString commandLine, int tableIndex = TABLE_INDEX_UI, bool wait = false, CommandCallback callback = nullptr);
        void sendRealtimeCommand(QString command);
        void sendRealtimeCommand(int command);
        void sendCommands(CommandSource source, QString commands, int tableIndex = -1);
        void sendCommands(CommandSource source, QStringList commands, int tableIndex = -1);
        // bool streamCommands(GCode &streamer);
        void clearCommandsAndQueue();
        void clearQueue();
        // void reset();
        // void unlock();
        // void home();
        // void probe();
        void resetGRBLConfiguration();
        // @TODO abort what?? find more self descriptive name, move to streamer??
        // void abort();
        // may be used to set connection for the first time, if m_connection is no null,
        // ReconnectingBehavior should be used instead!!

        bool setConnection(AbstractConnection *, bool force);
        bool startReconnecting(AbstractConnection *connection);
        AbstractStateBehavior *sb() const;
        // bool openConnection();
        AbstractConnection* connection();
        // void stopUpdatingState();
        // void startUpdatingState(int interval = -1);
        const SenderState& senderState() const { return m_senderState; }
        const MachineState& machineState() const { return m_machineState; }
        DeviceContext& deviceContext() { return m_deviceContext; }
        const DeviceContext& deviceContext() const { return m_deviceContext; }
        void setMachineType(MachineType type) { m_deviceContext.setMachineType(type); }
        QVector3D machinePos() const { return m_posTracker->machinePos(); }
        PositionTracker* positionTracker() { return m_posTracker; }
        // void sendStreamerCommandsUntilBufferIsFull();
        bool isMachineConfigurationReady() const;
        bool isSenderState(SenderState state) const;
        template<typename... Args>
        bool isSenderState(SenderState state, Args... args) const {
            return isSenderState(state) || isSenderState(args...);
        }

        Overrides* overrides() { return m_overrides; }
        CommandBuffer* commandBuffer() { return m_commandBuffer; }
        void queryMachineState();
        void queryMachineConfiguration();
        void processStateBehaviorTransition();

        AbstractStateBehavior* stateBehavior() const { return m_sbManager.current(); }
        StateBehaviorManager* stateBehaviorManager() { return &m_sbManager; }

        // Forwards a UI-supplied response to the active behavior. Routed via
        // the StateBehaviorManager so the UI doesn't have to know whether the
        // answering behavior is the current one or a UserPromptBehavior on top.
        void respondToPrompt(const QString &promptId, const QString &choiceId);
    private:
        AbstractConnection *m_connection = nullptr;
        Configuration *m_configuration;

        DeviceContext m_deviceContext;
        Overrides *m_overrides = nullptr;
        CommandBuffer *m_commandBuffer = nullptr;
        CommandScanner *m_commandScanner = nullptr;
        CommunicatorApi *m_comApi;

        // States
        SenderState m_senderState;
        MachineState m_machineState;
        StateBehaviorManager m_sbManager;

        QTimer m_startTime;

        PositionTracker* m_posTracker = nullptr;

        // Flags
        bool m_reseting;
        bool m_resetCompleted;
        // bool m_aborting;
        bool m_statusReceived;
        bool m_homing;
        bool m_spindleCW; // Spindle is rotating clockwise
        bool m_updateSpindleSpeed;
        bool m_updateParserState;

        // Stored parser params
        QString m_lastParserState; // response to $G
        QString m_storedParserState; // saved by storeParserState

        // Timers
        QTimer m_stateBehaviorTransitionTimer;
        QTimer *m_queryMachineStateTimer = nullptr;

        // Dictionary
        QMap<MachineState, QString> m_machineStateDictionary;

        //
        int m_lastAlarmCode = 0;

        bool execute(AbstractStateBehavior *statebehavior, bool force = false);
        void setSenderStateAndEmitSignal(SenderState);
        void setMachineStateAndEmitSignal(MachineState);
        void processOffsetsVars(QStringList response);
        static bool dataIsFloating(QString data);
        void processStatus(QString line);
        void processUnhandledResponse(QString data);
        void processMessage(QString data);
        void processAlarm(QString data);
        void processFeedSpindleSpeed(QString line);
        void processBuffersStatus(QString line);
        void processOverrides(QString line);
        void processMachineState(QString state);
        void processPinsState(QString line);
        void processSpindleState(QString line);
        void processWelcomeMessageDetected(QString message);
        void storeParserState();
        void restoreParserState();
        void completeTransfer();
        void resetStateVariables();
        void processDeviceConfiguration(QStringList response);
        void processGCodeParserState(CommandAttributes commandAttributes, QString response);
        void startQueryingMachineState();
        void stopQueryingMachineState();
        void onCommandBufferCompleted(CommandAttributes attributes, CmdStatus status, QStringList lines);

    private slots:
        // void onTimerStateQuery();
        void onConnectionLineReceived(QString);
        void onConnectionError(QString);
        void onConnectionStateChanged(ConnectionState state);
        void onStateRequestsTransition(AbstractStateBehavior *sb, AbstractStateBehavior *nsb,
                                       AbstractStateBehavior::TransitionKind kind);
        void onStateRequestsResume();
        void onStateError(AbstractStateBehavior *sb, QString message);

    signals:
        void responseReceived(QString command, int tableIndex, QString response);
        void statusReceived(QString status);
        void connectionChanged(AbstractConnection *connection);
        void connectionStateChanged(ConnectionState state);
        void alarm(int code);
        void welcomeMessageReceived(QString message);
        void senderStateReceived(SenderState state);
        void senderStateChanged(SenderState state);
        void machineStateChanged(MachineState state);
        void machineStatusReportReceived(MachineStatusReport report);
        void machineConfigurationReceived(PhysicalMachineConfiguration configuration);
        void machinePosChanged(QVector3D pos);
        void workPosChanged(QVector3D pos);
        void machineStateReceived(MachineState state);
        void machineStatusReceived(MachineState state);
        void spindleStateReceived(bool state);
        void pinStateReceived(PinState state);
        void parserStateReceived(QString state);
        void toolPositionReceived(QVector3D pos);
        void floodStateReceived(bool state);
        void buffersStatusReceived(int planerBufferBlocks, int serialBufferBytes);
        void commandResponseReceived(CommandAttributes commandAttributes);
        void commandSent(CommandAttributes commandAttributes);
        // @TODO what is the difference between spindleSpeedReceived and feedSpindleSpeedReceived??
        void spindleSpeedReceived(int spindleSpeed);
        void feedSpindleSpeedReceived(int feedRate, int spindleSpeed);
        void overridesReceived(int feedOverride, int spindleOverride, int rapidOverride);
        // @TODO how to do it better?? what signal should we attach to?
        void commandProcessed(int tableIndex, QString response);
        // @TODO aborted what?? find better name
        void aborted();
        void transferCompleted();
        void log(QString message);

        // Forwarded from the active behavior. UI is expected to display the
        // prompt and call respondToPrompt() with the user's choice id.
        void userPromptRequested(PromptSpec spec);
};

class CommunicatorApi : public QObject
{
    Q_OBJECT

    public:
        CommunicatorApi(Communicator *communicator) : QObject(), m_communicator(communicator) {}

        AbstractConnection *connection() { return m_communicator->m_connection; }
        const MachineState& machineState() const { return m_communicator->machineState(); }
        void queryMachineState() { m_communicator->queryMachineState(); }
        void processDeviceConfiguration(QStringList response) { m_communicator->processDeviceConfiguration(response); }
        void processOffsetsVars(QStringList response) { m_communicator->processOffsetsVars(response); }
        void processGCodeParserState(CommandAttributes commandAttributes, QString response) { m_communicator->processGCodeParserState(commandAttributes, response); }
        bool setConnection(AbstractConnection *connection, bool force) { return m_communicator->setConnection(connection, force); }
        int lastAlarmCode() const { return m_communicator->m_lastAlarmCode; }
        void startQueryingMachineState() { m_communicator->startQueryingMachineState(); }
        void stopQueryingMachineState() { m_communicator->stopQueryingMachineState(); }
        void queryMachineConfiguration() { m_communicator->queryMachineConfiguration(); }

        DeviceContext& deviceContext() { return m_communicator->m_deviceContext; }
        const DeviceContext& deviceContext() const { return m_communicator->m_deviceContext; }
        void setMachineType(MachineType type) { m_communicator->m_deviceContext.setMachineType(type); }
        QVector3D machinePos() { return m_communicator->machinePos(); }

        // Command
        SendCommandResult sendCommand(CommandSource source, QString commandLine, int tableIndex = TABLE_INDEX_UI, bool wait = false, CommandCallback callback = nullptr) {
            return m_communicator->sendCommand(source, commandLine, tableIndex, wait, callback);
        }
        void sendRealtimeCommand(QString command) { m_communicator->sendRealtimeCommand(command); }
        void sendRealtimeCommand(int command) { m_communicator->sendRealtimeCommand(command); }

        // Buffers — delegated to CommandBuffer
        void clearQueue() { m_communicator->commandBuffer()->clearQueue(); }
        void clearCommandsAndQueue() { m_communicator->commandBuffer()->clear(); }
        bool willOverflowBuffer(QString command) { return m_communicator->commandBuffer()->willOverflow(command); }
        bool isCommandBufferEmpty() { return m_communicator->commandBuffer()->isEmpty(); }
        bool isQueueEmpty() { return m_communicator->commandBuffer()->isQueueEmpty(); }
        CommandBuffer* commandBuffer() { return m_communicator->commandBuffer(); }
        QList<CommandAttributes>& commands() { return m_communicator->commandBuffer()->commands(); }
        int bufferLength() { return m_communicator->commandBuffer()->bufferLength(); }

    private:
        Communicator *m_communicator = nullptr;
};

#endif // COMMUNICATOR_H
