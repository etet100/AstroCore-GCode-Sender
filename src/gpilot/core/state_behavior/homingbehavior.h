// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef HOMINGBEHAVIOR_H
#define HOMINGBEHAVIOR_H

#include "statebehavior.h"

class HomingBehavior : public StateBehavior
{
    public:
        explicit HomingBehavior(QObject *parent = nullptr);
        QString description() override { return "Homing"; }
        Type type() const override { return Type::Homing; }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        void onAlarm(int code) override;
        bool doAction(const Action &action) override;

    protected:
        QString name() const override { return "Homing"; }

    private:
        bool m_homingStarted;
        bool m_homingCompleted;
};

#endif // HOMINGBEHAVIOR_H

