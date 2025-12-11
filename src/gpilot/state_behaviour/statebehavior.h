// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEBEHAVIOR_H
#define STATEBEHAVIOR_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "core/globals.h"
#include "action.h"
#include <functional>
#include <QMap>
#include <QPointer>

class CommunicatorApi;

class StateBehavior : public QObject
{
    Q_OBJECT

    public:
        enum Result : int {
            Ok = 0,
            ReturnCommandToQueue,
            Unhandled,
            Error,
            WaitForAsyncResult,
        };

        explicit StateBehavior(QObject *parent = nullptr);
        virtual QString description() = 0;

        bool eventsAttached() const { return m_eventsAttached; }
        void markEventsAttached() { m_eventsAttached = true; }

        virtual bool action(const Action &action);

        virtual bool onAboutToChange(StateBehavior *newState, bool forced) {
            Q_UNUSED(newState);
            Q_UNUSED(forced);
            return true;
        }

        // async exit means that onExit will emit exitCompleted signal when done
        // virtual bool exitAsync() { return false; };

        virtual void reset();
        virtual void unlock() {};

        StateBehavior* previous() const { return m_previous; }

        virtual Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) = 0;
        virtual Result onExit(StateBehavior *next = nullptr);

        virtual void onAlarm(int code) {
            qDebug() << "Alarm: " << code;
        }

        virtual void onMachineStateChanged(MachineState state) {
            Q_UNUSED(state);
        }

        using StateResponseCallback = std::function<void(MachineState)>;

        virtual void onMachineState(MachineState state) {
            for (auto& cbk : m_stateResponseCallbacks) {
                cbk(state);
            }
            m_stateResponseCallbacks.clear();
        }

        // returns true if the response was handled and should not be processed further, for example
        // passed to onCommandResponse.
        virtual Result onRawResponse(QString response);

        // returns true if the response was handled and should not be processed further.
        virtual Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) {
            Q_UNUSED(command);
            Q_UNUSED(commandAttributes);
            Q_UNUSED(cmdStatus);
            Q_UNUSED(response);
            Q_UNUSED(fullResponse);

            return Result::Unhandled;
        }

        virtual void onConnectionStateChanged(ConnectionState state) {
            Q_UNUSED(state);
        }

        void waitForStateResponse(StateResponseCallback callback);

    signals:
        void transition(StateBehavior *state, StateBehavior *newState);
        void error(StateBehavior *state, QString message);
        void logSignal(QString message);
        void asyncCompleted();

    protected:
        StateBehavior *m_previous = nullptr;
        QPointer<CommunicatorApi> m_communicator = nullptr;
        QTimer *m_timer = nullptr;
        QList<StateResponseCallback> m_stateResponseCallbacks;

        virtual QString name() const = 0;
        void stopTimer();
        void log(QString message, QStringList context = QStringList());
        void log(QString message, std::initializer_list<QString> context);
        // This is something we will need in almost every behavior
        bool dataIsReset(QString data);

        virtual bool doAction(const Action &action) {
            Q_UNUSED(action);
            return false;
        }

        QString enrichErrorMessage(QString message);
        static const QMap<int, QString> ERRORS;
        static const QMap<int, QString> ALARMS;

    private:
        bool m_eventsAttached = false; // used by Communicator
};

#endif // STATEBEHAVIOR_H
