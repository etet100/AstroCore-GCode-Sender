// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef RUNNINGBEHAVIOR_H
#define RUNNINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/gcode/gcode.h"

class RunningBehavior : public StateBehavior
{
    public:
        enum class RunningStage {
            Unknown,
            Resuming,
            Running,
            NoMoreCommands,
        };

        explicit RunningBehavior(GCode &program, QObject *parent = nullptr);
        QString description() override { return "Running"; }
        QSet<Action::Type> availableActions() const override {
            return { Action::PauseResume, Action::Stop };
        }
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        void onAlarm(int code) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        // Running-specific methods
        void handleFeedOverride(int percentage);
        void handleSpindleOverride(int percentage);

    protected:
        QString name() const override { return "Running"; }
        bool doAction(const Action &action) override;

    private:
        RunningStage m_stage;
        int m_feedOverride;
        int m_spindleOverride;
        GCode &m_program;
        bool m_pause = false;

        void sendStreamerCommandsUntilBufferIsFull();
        void pause();
};

#endif // RUNNINGBEHAVIOR_H
