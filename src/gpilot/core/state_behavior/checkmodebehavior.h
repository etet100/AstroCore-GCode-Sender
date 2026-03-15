// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CHECKMODEBEHAVIOR_H
#define CHECKMODEBEHAVIOR_H

#include "statebehavior.h"
#include "core/gcode/gcode.h"

class CheckModeBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit CheckModeBehavior(GCode &program, QObject *parent = nullptr);
        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        void onAlarm(int code) override;

    protected:
        QString name() const override { return "CheckMode"; }
        bool doAction(const Action &action) override;

    private:
        GCode &m_program;
        bool m_stopped;

        void sendStreamerCommandsUntilBufferIsFull();
        void stop();
};

#endif // CHECKMODEBEHAVIOR_H
