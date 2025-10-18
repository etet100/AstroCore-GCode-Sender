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

class Communicator;

class StateBehavior : public QObject
{
    Q_OBJECT

    public:
        explicit StateBehavior(QObject *parent = nullptr);
        virtual QString name() = 0;
        virtual bool execute(const Action &action) {
            Q_UNUSED(action);
        };
        virtual bool isActionAllowed(const Action &action) {
            Q_UNUSED(action);
            return false;
        };
        virtual void reset();
        virtual void unlock() {};
        virtual bool isJoggingAllowed() { return false; };
        virtual bool isHomingAllowed() { return false; };
        StateBehavior* previous() const { return m_previous; }
        virtual void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) = 0;

        virtual void onExit(StateBehavior *next = nullptr);
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
        virtual void onCommandResponse(QString command, CommandAttributes commandAttributes, QStringList response) {
            Q_UNUSED(commandAttributes);
            onCommandResponse(command, response);
        }
        virtual void onConnectionStateChanged(ConnectionState state) {
            Q_UNUSED(state);
        };

    signals:
        void transition(StateBehavior *state, StateBehavior *newState);
        void error(StateBehavior *state, QString message);

    // public slots:
    //     virtual void onConnectionStateChanged(ConnectionState state) {
    //         Q_UNUSED(state);
    //     };

    protected:
        StateBehavior *m_previous = nullptr;
        Communicator *m_communicator = nullptr;
        QTimer *m_timer = nullptr;

        void stopTimer();
};

#endif // STATEBEHAVIOR_H
