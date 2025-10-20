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

class Communicator;

class StateBehavior : public QObject
{
    Q_OBJECT

    public:
        explicit StateBehavior(QObject *parent = nullptr);
        virtual QString name() = 0;

        bool eventsAttached() const { return m_eventsAttached; }
        void markEventsAttached() { m_eventsAttached = true; }

        virtual bool execute(const Action &action) {
            Q_UNUSED(action);
        }

        virtual bool isActionAllowed(const Action &action) {
            Q_UNUSED(action);
            return false;
        }

        virtual bool isNewStateAllowed(StateBehavior *newState) {
            Q_UNUSED(newState);
            return true;
        }

        // async exit means that onExit will emit exitCompleted signal when done
        virtual bool exitAsync() { return false; };

        virtual void reset();
        virtual void unlock() {};
        virtual bool isJoggingAllowed() { return false; };
        virtual bool isHomingAllowed() { return false; };
        StateBehavior* previous() const { return m_previous; }
        virtual void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) = 0;

        virtual void onExit(StateBehavior *next = nullptr);

        virtual void onAlarm(int code) {
            qDebug() << "Alarm: " << code;
        }

        virtual void onDeviceStateChanged(DeviceState state) {
            Q_UNUSED(state);
        }

        using StateResponseCallback = std::function<void(DeviceState)>;

        virtual void onDeviceState(DeviceState state) {
            for (auto cbk : m_stateResponseCallbacks) {
                cbk(state);
            }
            m_stateResponseCallbacks.clear();
        }

        // returns true if the response was handled and should not be processed further, for example
        // passed to onCommandResponse.
        virtual bool onRawResponse(QString response) {
            Q_UNUSED(response);

            return false;
        }

        // returns true if the response was handled and should not be processed further.
        virtual bool onCommandResponse(QString command, QString response, QStringList fullResponse) {
            Q_UNUSED(command);
            Q_UNUSED(response);
            Q_UNUSED(fullResponse);

            return false;
        }

        // returns true if the response was handled and should not be processed further.
        virtual bool onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse) {
            Q_UNUSED(commandAttributes);

            return onCommandResponse(command, response, fullResponse);
        }

        virtual void onConnectionStateChanged(ConnectionState state) {
            Q_UNUSED(state);
        }

        void waitForStateResponse(StateResponseCallback callback);

    signals:
        void transition(StateBehavior *state, StateBehavior *newState);
        void error(StateBehavior *state, QString message);
        void logSignal(QString message);
        void exitCompleted();

    protected:
        StateBehavior *m_previous = nullptr;
        QPointer<Communicator> m_communicator = nullptr;
        QTimer *m_timer = nullptr;
        QList<StateResponseCallback> m_stateResponseCallbacks;

        void stopTimer();
        void log(QString message);

    private:
        bool m_eventsAttached = false; // used by Communicator
};

#endif // STATEBEHAVIOR_H
