// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef BABBA109_70DB_4039_9E49_2771C68D2FDA
#define BABBA109_70DB_4039_9E49_2771C68D2FDA
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef ALARMBEHAVIOR_H
#define ALARMBEHAVIOR_H

#include "statebehavior.h"

class AlarmBehavior : public StateBehavior
{
    public:
        explicit AlarmBehavior(StateBehavior *previous, int alarmCode = 0, QObject *parent = nullptr);
        QString name() override { return "Alarm: " + m_alarmMessage; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return true; } // Homing is usually allowed in alarm state (may help exit this state)
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onCommandResponse(QString command, QStringList response) override;
        void onConnectionStateChanged(ConnectionState state) override;

        // Alarm-specific methods
        void unlockAlarm(); // Method to send unlock command ($X)

    private:
        int m_alarmCode;
        QString m_alarmMessage;
        void setAlarmMessage();
};

#endif // ALARMBEHAVIOR_H


#endif /* BABBA109_70DB_4039_9E49_2771C68D2FDA */
