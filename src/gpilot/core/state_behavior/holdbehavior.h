// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef HOLDBEHAVIOR_H
#define HOLDBEHAVIOR_H

#include "statebehavior.h"

class HoldBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        enum class HoldSource {
            UserRequest,    // Manual feed hold by user (!)
            Door,           // Safety door triggered
            Emergency       // Emergency stop
        };

        explicit HoldBehavior(HoldSource source = HoldSource::UserRequest, QObject *parent = nullptr);
        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

    protected:
        QString name() const override { return "Hold"; }
        bool doAction(const Action &action) override;

    private:
        HoldSource m_source;
        void resume();
};

#endif // HOLDBEHAVIOR_H
