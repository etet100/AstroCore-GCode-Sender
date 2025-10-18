// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef ALARMBEHAVIOR_H
#define ALARMBEHAVIOR_H

#include "statebehavior.h"

class AlarmBehavior : public StateBehavior
{
    public:
        explicit AlarmBehavior(int alarmCode = 0, QObject *parent = nullptr);
        QString name() override { return "Alarm: " + m_alarmMessage; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return true; } // Homing is usually allowed in alarm state (may help exit this state)
        void onDeviceStateChanged(DeviceState state) override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onCommandResponse(QString command, QStringList response) override;
        // void onConnectionStateChanged(ConnectionState state) override;
        void unlock() override;
        bool execute(const Action &action) override;
        bool isActionAllowed(const Action &action) override;

    private:
        int m_alarmCode;
        QString m_alarmMessage;
        void setAlarmMessage();

        // StateBehavior interface
};

#endif // ALARMBEHAVIOR_H
