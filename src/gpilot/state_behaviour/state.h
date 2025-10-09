// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATE_H
#define STATE_H

#include <QObject>
#include "io/connection/connection.h"

class Communicator;

class State : public QObject
{
    Q_OBJECT

    public:
        explicit State(State *previous, QObject *parent = nullptr);
        virtual QString name() = 0;
        virtual bool isJoggingAllowed() { return false; };
        virtual bool isHomingAllowed() { return false; };
        State* previous() const { return m_previous; }
        virtual void onEntry(Communicator *communicator, State *previous = nullptr);

        virtual void onExit() {};
        virtual void onAlarm(int code) {
            qDebug() << "Alarm: " << code;
        };
        virtual void onDeviceStateChanged(DeviceState state) {
            Q_UNUSED(state);
        };
        virtual void onCommandResponse(QString command, QStringList response) {
            Q_UNUSED(command);
            Q_UNUSED(response);
        };

    signals:
        void transition(State *state, State *newState);
        void error(State *state, QString message);

    public slots:
        virtual void onConnectionStateChanged(ConnectionState state) {
            Q_UNUSED(state);
        };

    protected:
        State *m_previous;
        Communicator *m_communicator;
};

#endif // STATE_H
