// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef RUNNINGBEHAVIOR_H
#define RUNNINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/gcode/gcode.h"

class RunningBehavior : public StateBehavior
{
    public:
        explicit RunningBehavior(GCode &program, QObject *parent = nullptr);
        QString name() override { return "Running"; }
        void onMachineStateChanged(MachineState state) override;
        bool onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse) override;
        void onAlarm(int code) override;
        bool action(const Action &action) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        // Running-specific methods
        void handleFeedOverride(int percentage);
        void handleSpindleOverride(int percentage);

    private:
        int m_feedOverride;
        int m_spindleOverride;
        GCode &m_program;
        bool m_pause;

        void sendStreamerCommandsUntilBufferIsFull();
        void pause();
        void resume();
};

#endif // RUNNINGBEHAVIOR_H
