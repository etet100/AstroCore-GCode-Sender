// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef BCFFC5C6_E86C_4C65_9BFA_24FC7689E3A5
#define BCFFC5C6_E86C_4C65_9BFA_24FC7689E3A5
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEALARM_H
#define STATEALARM_H

#include "state.h"

class StateAlarm : public State
{
    public:
        explicit StateAlarm(State *previous, int alarmCode = 0, QObject *parent = nullptr);
        QString name() override { return "Alarm: " + m_alarmMessage; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return true; } // Homing is typically allowed to clear alarm state
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
        void onCommandResponse(QString command, QStringList response) override;
        void onConnectionStateChanged(ConnectionState state) override;

    private:
        int m_alarmCode;
        QString m_alarmMessage;
        void setAlarmMessage();
};

#endif // STATEALARM_H


#endif /* BCFFC5C6_E86C_4C65_9BFA_24FC7689E3A5 */
