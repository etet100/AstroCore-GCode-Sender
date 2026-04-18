// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef ABSTRACTSTATEBEHAVIOR_H
#define ABSTRACTSTATEBEHAVIOR_H

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
#include <QCoroTask>

class CommunicatorApi;

class AbstractStateBehavior : public QObject
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
            ScanTableError,
            ToolChange,
        };

        // Transition semantics for the `transition` signal.
        // Replace: current behavior ends; the suspended stack is flushed.
        // Suspend: current behavior is pushed onto the suspended stack and
        //          can be brought back via `resumePrevious`.
        enum class TransitionKind {
            Replace,
            Suspend,
        };

        // Context handed to a behavior's onEntry. previousType is nullopt on the
        // very first state activation. data is the exit payload of the previous
        // behavior (or the resuming one, for Resume transitions).
        struct EntryContext {
            std::optional<Type> previousType;
            QVariantMap data;
        };

        explicit AbstractStateBehavior(QObject *parent = nullptr);
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

        bool is(Type t) const {
            return type() == t;
        }

        template<typename... Args>
        bool isOneOf(Args... types) const {
            return ((type() == types) || ...);
        }

        virtual bool onAboutToChange(AbstractStateBehavior *newState, bool forced) {
            Q_UNUSED(newState);
            Q_UNUSED(forced);
            return true;
        }

        // async exit means that onExit will emit exitCompleted signal when done
        // virtual bool exitAsync() { return false; };

        // NVI: these run base setup/cleanup and then call the protected doOn* hook.
        // Derived classes override the hook, not these methods.
        Result onEntry(CommunicatorApi *communicator, const EntryContext &ctx = {});
        Result onExit(AbstractStateBehavior *next = nullptr);

        // Payload accumulated during the behavior's lifetime. The StateBehaviorManager
        // reads it at transition/resume time and passes it to the successor as
        // EntryContext::data.
        const QVariantMap& exitData() const { return m_exitData; }

        // Default: logs the alarm, sets m_alarmOccurred/m_alarmCode, and wakes any
        // co_awaiting coroutine via commandResponseReceived.
        virtual void onAlarm(int code);

        // Default: emits machineStateChangedSignal for coroutine co_await.
        virtual void onMachineStateChanged(MachineState state);

        using StateResponseCallback = std::function<void(MachineState)>;

        // NVI: dispatches m_stateResponseCallbacks then calls doOnMachineState hook.
        void onMachineState(MachineState state);

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
        void transition(AbstractStateBehavior *state, AbstractStateBehavior *newState,
                        AbstractStateBehavior::TransitionKind kind = AbstractStateBehavior::TransitionKind::Replace);
        void resumePrevious();
        void error(AbstractStateBehavior *state, QString message);
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
        void commandResponseReceived(AbstractStateBehavior::CommandResult result);
        void machineStateSignal(MachineState state);
        void machineStateChangedSignal(MachineState state);

    protected:
        QPointer<CommunicatorApi> m_communicator = nullptr;
        QTimer *m_timer = nullptr;
        QVariantMap m_exitData;
        std::optional<Type> m_previousType;  // set by base onEntry from EntryContext

        // Helpers to accumulate the exit payload during the behavior's work.
        void setExitValue(const QString &key, const QVariant &value) { m_exitData.insert(key, value); }
        void setExitValue(const QVariantMap &values) { m_exitData.insert(values); }
        void clearExitData() { m_exitData.clear(); }

        bool m_alarmOccurred = false;
        int m_alarmCode = 0;

        // Coroutine helpers: send a command and wait for its specific response by commandIndex.
        QCoro::Task<std::optional<CommandResult>> sendAndAwait(const QString &command,
                                                                std::chrono::milliseconds timeout);
        QCoro::Task<std::optional<CommandResult>> awaitResponse(int commandIndex,
                                                                 std::chrono::milliseconds timeout);
        // Waits until the machine reaches a state matching predicate, or until timeout.
        // Returns the matching state, or nullopt on timeout.
        QCoro::Task<std::optional<MachineState>> awaitMachineState(
            std::function<bool(MachineState)> predicate,
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

        // No need to call base implementation of doAction if you override it
        virtual bool doAction(const Action &action) {
            Q_UNUSED(action);
            return false;
        }

        // NVI hooks — override these instead of the public on*() methods.
        // The base class runs setup before doOnEntry and cleanup before doOnExit;
        // doOnMachineState fires after m_stateResponseCallbacks are dispatched.
        virtual Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) {
            Q_UNUSED(communicator);
            Q_UNUSED(ctx);
            return Result::Ok;
        }
        virtual Result doOnExit(AbstractStateBehavior *next) {
            Q_UNUSED(next);
            return Result::Ok;
        }
        virtual void doOnMachineState(MachineState state) { Q_UNUSED(state); }

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
        void disconnectAction();
};

#endif // ABSTRACTSTATEBEHAVIOR_H
