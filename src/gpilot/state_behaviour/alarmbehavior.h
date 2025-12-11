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
        QString description() override;
        void onMachineStateChanged(MachineState state) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

    protected:
        QString name() const override { return "AlarmBehavior"; }
        bool doAction(const Action &action) override;

    private:
        int m_alarmCode;
        QString m_alarmMessage;
        void setAlarmMessage();
        void unlock() override;
};

#endif // ALARMBEHAVIOR_H
