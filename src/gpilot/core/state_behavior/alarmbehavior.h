// This file is a part of "G-Pilot GCode Sender" application.
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
        Type type() const override { return Type::Alarm; }
        QSet<Action::Type> availableActions() const override {
            return {
                Action::Reset,
                Action::Unlock,
                Action::Disconnect,
            };
        }
        void onMachineStateChanged(MachineState state) override;
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

    protected:
        QString name() const override { return "Alarm"; }
        bool doAction(const Action &action) override;

    private:
        int m_alarmCode;
        QString m_alarmMessage;
        void setAlarmMessage();
        void unlock();
};

#endif // ALARMBEHAVIOR_H
