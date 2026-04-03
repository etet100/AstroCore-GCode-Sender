// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEBEHAVIOR_H
#define STATEBEHAVIOR_H

#include <QObject>
#include <QDebug>
#include <QHash>
#include <QSet>
#include <QTimer>
#include <QVariantMap>
#include "core/globals.h"
#include "action.h"
#include <functional>
#include <QMap>
#include <QPointer>
#include <optional>
#include <chrono>
#include <qcorotask.h>

class CommunicatorApi;

class StateBehavior : public QObject
{
    Q_OBJECT

    public:
        // Common type for delivering a complete command response to a coroutine.
        struct CommandResult {
            QString command;
            CommandAttributes attributes;
            CmdStatus status;
            QString response;
            QStringList fullResponse;
        };

        enum Result : int {
            Ok = 0,
            ReturnCommandToQueue,
            Unhandled,
            Error,
            WaitForAsyncResult,
        };

        enum class Type {
            Alarm,
            CheckMode,
            Connecting,
            Disconnection,
            Error,
            ExternalProcess,
            GoTo,
            Handshake,
            Hold,
            Homing,
            Idle,
            Initialization,
            Jogging,
            Pause,
            Probing,
            Reconnecting,
            Reset,
            Running,
            ScanTable,
            ToolChange,
        };

        explicit StateBehavior(QObject *parent = nullptr);
        virtual QString description() = 0;
        virtual Type type() const = 0;

        bool eventsAttached() const { return m_eventsAttached; }
        void markEventsAttached() { m_eventsAttached = true; }

        bool action(const Action &action);

        // Returns the set of actions that this state accepts.
        // UI uses this to enable / disable controls.
        virtual QSet<Action::Type> availableActions() const {
            return { Action::Type::Reset };
        }

        bool canExecute(Action::Type type) const {
            return availableActions().contains(type);
        }

        virtual bool onAboutToChange(StateBehavior *newState, bool forced) {
            Q_UNUSED(newState);
            Q_UNUSED(forced);
            return true;
        }

        // async exit means that onExit will emit exitCompleted signal when done
        // virtual bool exitAsync() { return false; };

        StateBehavior* previous() const { return m_previous; }

        virtual Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) = 0;
        virtual Result onExit(StateBehavior *next = nullptr);

        // Default: logs the alarm, sets m_alarmOccurred/m_alarmCode, and wakes any
        // co_awaiting coroutine via commandResponseReceived.
        virtual void onAlarm(int code);

        // Default: emits machineStateChangedSignal for coroutine co_await.
        virtual void onMachineStateChanged(MachineState state);

        using StateResponseCallback = std::function<void(MachineState)>;

        // if overriden, do not forget to call StateBehavior::onMachineState(state)
        virtual void onMachineState(MachineState state);

        // returns true if the response was handled and should not be processed further, for example
        // passed to onCommandResponse.
        virtual Result onRawResponse(QString response);

        // Default: emits commandResponseReceived for coroutine co_await and returns Ok.
        // Behaviors that need custom response handling should override this.
        virtual Result onCommandResponse(QString command, CommandAttributes commandAttributes,
                                         CmdStatus cmdStatus, QString response, QStringList fullResponse);

        virtual void onConnectionStateChanged(ConnectionState state) {
            Q_UNUSED(state);
        }

        // Registers a callback to fire when the machine reaches targetState (Unknown = any state).
        // If milliseconds > 0, fires callback(MachineState::Unknown) on timeout.
        // Returns an ID that can be passed to clearWaitForStateResponse() to cancel.
        int waitForStateResponse(StateResponseCallback callback,
                                 MachineState targetState = MachineState::Unknown,
                                 int milliseconds = 0);
        void clearWaitForStateResponse(int id);

    signals:
        void transition(StateBehavior *state, StateBehavior *newState);
        void error(StateBehavior *state, QString message);
        void logSignal(QString message);
        void asyncCompleted();

        // Progress of the current operation (e.g. scanning, running).
        // Emit when the behavior has a natural concept of progress.
        void progressChanged(int current, int total);

        // Generic event for behavior-specific notifications that don't fit
        // into other signals. Use a short camelCase type string and a flat
        // QVariantMap with the relevant data.
        void stateEvent(QString type, QVariantMap data);

        // QCoro bridge signals — emitted from the default onCommandResponse / onMachineStateChanged
        // implementations so that coroutine-based behaviors can co_await them.
        void commandResponseReceived(StateBehavior::CommandResult result);
        void machineStateChangedSignal(MachineState state);

    protected:
        StateBehavior *m_previous = nullptr;
        QPointer<CommunicatorApi> m_communicator = nullptr;
        QTimer *m_timer = nullptr;

        bool m_alarmOccurred = false;
        int m_alarmCode = 0;

        // Coroutine helpers: send a command and wait for its specific response by commandIndex.
        QCoro::Task<std::optional<CommandResult>> sendAndAwait(const QString &command,
                                                                std::chrono::milliseconds timeout);
        QCoro::Task<std::optional<CommandResult>> awaitResponse(int commandIndex,
                                                                 std::chrono::milliseconds timeout);

        struct StateResponseEntry {
            int id;
            MachineState targetState;
            int timerId;
            StateResponseCallback callback;
        };
        QList<StateResponseEntry> m_stateResponseCallbacks;

        virtual QString name() const = 0;
        bool handleMachineConfigurationActions(const Action &action);
        bool handleSaveMachineConfigurationParamAction(const Action &action);
        void stopTimer();
        void log(QString message, QStringList context = QStringList());
        void log(QString message, std::initializer_list<QString> context);
        // This is something we will need in almost every behavior
        // bool dataIsReset(QString data);

        // No need to call base implementation of doAction if you override it
        virtual bool doAction(const Action &action) {
            Q_UNUSED(action);
            return false;
        }

        bool transitionToPreviousState();

        int setTimeout(int milliseconds, std::function<void()> callback = nullptr);
        void clearTimeout(int id);
        void clearAllTimeouts();

        QString enrichErrorMessage(QString message);
        static const QMap<int, QString> ERRORS;
        static const QMap<int, QString> ALARMS;

    private:
        QHash<int, QTimer*> m_timers;
        bool m_eventsAttached = false; // used by Communicator
        void reset();
};

#endif // STATEBEHAVIOR_H
