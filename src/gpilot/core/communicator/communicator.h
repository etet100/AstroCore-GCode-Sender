#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

#include "core/globals.h"
#include "core/gcode/gcode.h"
#include "core/config/configuration.h"
#include "io/connection/connection.h"
#include "scripting/scriptvars.h"
#include "core/machine/physicalmachineconfiguration.h"
#include "core/jogger/jogger.h"
#include "state_behaviour/statebehavior.h"
#include <QTimer>
#include <QPointer>

class Communicator : public QObject
{
    friend class Jogger;
    friend class CommunicatorApi;

    Q_OBJECT

    public:
        Communicator(
            QObject *parent,
            Connection *connection,
            Configuration *configuration
        );
        SendCommandResult sendCommand(CommandSource source, QString commandLine, int tableIndex = TABLE_INDEX_UI, bool wait = false, CommandCallback callback = nullptr);
        void sendRealtimeCommand(QString command);
        void sendRealtimeCommand(int command);
        void sendCommands(CommandSource source, QString commands, int tableIndex = -1);
        void sendCommands(CommandSource source, QStringList commands, int tableIndex = -1);
        // bool streamCommands(GCode &streamer);
        void clearCommandsAndQueue();
        void clearQueue();
        void reset();
        void unlock();
        void home();
        void probe();
        // @TODO abort what?? find more self descriptive name, move to streamer??
        void abort();
        // may be used to set connection for the first time, if m_connection is no null,
        // ReconnectingBehavior should be used instead!!
        bool setConnection(Connection *, bool force);
        StateBehavior *sb() const;
        // bool openConnection();
        Connection* connection();
        // void stopUpdatingState();
        // void startUpdatingState(int interval = -1);
        const SenderState& senderState() const { return m_senderState; }
        const MachineState& machineState() const { return m_machineState; }
        PhysicalMachineConfiguration& machineConfiguration() const { return *m_machineConfiguration; }
        QVector3D machinePos() const { return m_machinePos; }
        // void sendStreamerCommandsUntilBufferIsFull();
        bool isMachineConfigurationReady() const;
        bool isSenderState(SenderState state) const;
        template<typename... Args>
        bool isSenderState(SenderState state, Args... args) const {
            return isSenderState(state) || isSenderState(args...);
        }
        // void probe();
        bool execute(StateBehavior *stateBehaviour, bool force = false);

        // @TODO to be removed!! another local timer? how it works??
        void processConnectionTimer();
        Jogger& jogger() { return m_jogger; }
        void queryMachineState();
        void queryMachineConfiguration();
        void processStateBehaviorTransition();

        StateBehavior* stateBehavior() const { return m_sb.data(); }
    private:
        static const int BUFFERLENGTH = 127;

        Connection *m_connection = nullptr;;
        Configuration *m_configuration;
        GCode *m_streamer = nullptr;
        PhysicalMachineConfiguration *m_machineConfiguration = nullptr;
        Jogger m_jogger;
        CommunicatorApi *m_comApi;

        // Queues
        QList<CommandAttributes> m_commands;
        QList<CommandQueue> m_queue;

        // States
        SenderState m_senderState;
        MachineState m_machineState;
        QPointer<StateBehavior> m_sb = nullptr;
        QPointer<StateBehavior> m_nsb = nullptr; // next state behavior to be set

        ScriptVars m_storedVars;

        QTimer m_startTime;

        // Coordinates
        QVector3D m_machinePos;
        QVector3D m_workOffset;

        // Flags
        bool m_reseting;
        bool m_resetCompleted;
        bool m_aborting;
        bool m_statusReceived;
        bool m_homing;
        bool m_spindleCW; // Spindle is rotating clockwise
        bool m_updateSpindleSpeed;
        bool m_updateParserState;

        // Indices
        // int m_probeIndex;
        int m_commandIndex = 0;

        // Stored parser params
        QString m_lastParserState; // response to $G
        QString m_storedParserState; // saved by storeParserState

        // Timers
        QTimer *m_queryMachineStateTimer = nullptr;

        // Dictionary
        QMap<MachineState, QString> m_machineStateDictionary;

        //
        int m_lastAlarmCode = 0;

        void setSenderStateAndEmitSignal(SenderState);
        void setMachineStateAndEmitSignal(MachineState);
        void restoreOffsets();
        int bufferLength();
        void processOffsetsVars(QStringList response);
        static bool dataIsFloating(QString data);
        static bool dataIsEnd(QString data);
        static bool dataIsReset(QString data);
        bool compareCoordinates(double x, double y, double z);
        double toMetric(double value);
        double toInches(double value);
        void processStatus(QString line);
        bool processCommandResponse(QString data);
        void processUnhandledResponse(QString data);
        void processMessage(QString data);
        void processAlarm(QString data);
        void processFeedSpindleSpeed(QString line);
        void processBuffersStatus(QString line);
        void processOverrides(QString line);
        void processWorkOffset(QString line);
        void processMachinePosition(QString line);
        void processWorkPosition(QString line);
        void processMachineState(QString state);
        void processPinsState(QString line);
        void processSpindleState(QString line);
        void processNewToolPosition();
        void processWelcomeMessageDetected(QString message);
        void storeParserState();
        void restoreParserState();
        void completeTransfer();
        void resetStateVariables();
        void processDeviceConfiguration(QStringList response);
        void processGCodeParserState(CommandAttributes commandAttributes, QString response);
        bool finalizeExecute(StateBehavior *sb);
        bool willOverflowBuffer(QString command);
        void startQueryingMachineState();
        void stopQueryingMachineState();

    private slots:
        // void onTimerStateQuery();
        void onConnectionLineReceived(QString);
        void onConnectionError(QString);
        void onConnectionStateChanged(ConnectionState state);
        void onStateRequestsTransition(StateBehavior *sb, StateBehavior *nsb);
        void onStateError(StateBehavior *sb, QString message);

    signals:
        void responseReceived(QString command, int tableIndex, QString response);
        void statusReceived(QString status);
        void connectionChanged(Connection *connection);
        void alarm(int code);
        void welcomeMessageReceived(QString message);
        void senderStateReceived(SenderState state);
        void senderStateChanged(SenderState state);
        void machineStateChanged(MachineState state);
        void machineConfigurationReceived(PhysicalMachineConfiguration configuration);
        void machinePosChanged(QVector3D pos);
        void workPosChanged(QVector3D pos);
        void machineStateReceived(MachineState state);
        void spindleStateReceived(bool state);
        void pinStateReceived(QString state);
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
        void stateBehaviorChanged(StateBehavior *sb);
        void log(QString message);
};

class CommunicatorApi : public QObject
{
    Q_OBJECT

    public:
        CommunicatorApi(Communicator *communicator) : QObject(), m_communicator(communicator) {}

        Connection *connection() { return m_communicator->m_connection; }
        void queryMachineState() { m_communicator->queryMachineState(); }
        void processDeviceConfiguration(QStringList response) { m_communicator->processDeviceConfiguration(response); }
        void processOffsetsVars(QStringList response) { m_communicator->processOffsetsVars(response); }
        void processConnectionTimer() { m_communicator->processConnectionTimer(); }
        void processGCodeParserState(CommandAttributes commandAttributes, QString response) { m_communicator->processGCodeParserState(commandAttributes, response); }
        bool setConnection(Connection *connection, bool force) { return m_communicator->setConnection(connection, force); }
        int lastAlarmCode() const { return m_communicator->m_lastAlarmCode; }
        void startQueryingMachineState() { m_communicator->startQueryingMachineState(); }
        void stopQueryingMachineState() { m_communicator->stopQueryingMachineState(); }
        void queryMachineConfiguration() { m_communicator->queryMachineConfiguration(); }

        // Command
        SendCommandResult sendCommand(CommandSource source, QString commandLine, int tableIndex = TABLE_INDEX_UI, bool wait = false, CommandCallback callback = nullptr) {
            return m_communicator->sendCommand(source, commandLine, tableIndex, wait, callback);
        }
        void sendRealtimeCommand(QString command) { m_communicator->sendRealtimeCommand(command); }
        void sendRealtimeCommand(int command) { m_communicator->sendRealtimeCommand(command); }

        // Buffers
        void clearQueue() { m_communicator->clearQueue(); }
        void clearCommandsAndQueue() { m_communicator->clearCommandsAndQueue(); }
        bool willOverflowBuffer(QString command) { return m_communicator->willOverflowBuffer(command); }
        bool isCommandBufferEmpty() { return m_communicator->m_commands.isEmpty(); }
        bool isQueueEmpty() { return m_communicator->m_queue.isEmpty(); }
        QList<CommandAttributes>& commands() { return m_communicator->m_commands; }
        int bufferLength() { return m_communicator->bufferLength(); }

    private:
        Communicator *m_communicator = nullptr;
};

#endif // COMMUNICATOR_H
