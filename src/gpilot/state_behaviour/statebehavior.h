// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEBEHAVIOR_H
#define STATEBEHAVIOR_H

#include <QObject>
#include <QDebug>
#include "../globals.h"

class Communicator;

class StateBehavior : public QObject
{
    Q_OBJECT

    public:
        explicit StateBehavior(StateBehavior *previous, QObject *parent = nullptr);
        virtual QString name() = 0;
        virtual bool isJoggingAllowed() { return false; };
        virtual bool isHomingAllowed() { return false; };
        StateBehavior* previous() const { return m_previous; }
        virtual void onEntry(Communicator *communicator, StateBehavior *previous = nullptr);

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
        void transition(StateBehavior *state, StateBehavior *newState);
        void error(StateBehavior *state, QString message);

    public slots:
        virtual void onConnectionStateChanged(ConnectionState state) {
            Q_UNUSED(state);
        };

    protected:
        StateBehavior *m_previous;
        Communicator *m_communicator;
};

#endif // STATEBEHAVIOR_H
