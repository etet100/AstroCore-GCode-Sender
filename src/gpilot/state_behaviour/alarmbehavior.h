// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef ALARMBEHAVIOR_H
#define ALARMBEHAVIOR_H

#include "statebehavior.h"

class AlarmBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit AlarmBehavior(int alarmCode = 0, QObject *parent = nullptr);
        QString name() override { return "Alarm: " + m_alarmMessage; }

        void onMachineStateChanged(MachineState state) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        bool onCommandResponse(QString command, QString response, QStringList fullResponse) override;
        void unlock() override;
        bool action(const Action &action) override;
        bool isActionAllowed(const Action &action) override;

    private:
        int m_alarmCode;
        QString m_alarmMessage;
        void setAlarmMessage();
};

#endif // ALARMBEHAVIOR_H
