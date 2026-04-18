// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef RUNNINGBEHAVIOR_H
#define RUNNINGBEHAVIOR_H

#include "abstractstatebehavior.h"
#include "core/gcode/gcode.h"
#include "core/communicator/commandscanner.h"

class RunningBehavior : public AbstractStateBehavior
{
    public:
        enum class Stage {
            Unknown,
            Resuming,
            Running,
            Pausing,
            NoMoreCommands,
            Aborting,
        };

        explicit RunningBehavior(GCode &program, QObject *parent = nullptr);
        QString description() override { return "Running"; }
        Type type() const override { return Type::Running; }
        QSet<Action::Type> availableActions() const override {
            return {
                Action::Reset,
                Action::Pause,
                Action::Abort
            };
        }
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        void onAlarm(int code) override;
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        // Running-specific methods
        void handleFeedOverride(int percentage);
        void handleSpindleOverride(int percentage);

    protected:
        QString name() const override { return "Running"; }
        bool doAction(const Action &action) override;

    private:
        Stage m_stage;
        int m_feedOverride;
        int m_spindleOverride;
        GCode &m_program;
        CommandScanner m_commandScanner;
        int m_lastLookAheadIndex = -1;
        void sendStreamerCommandsUntilBufferIsFull();
        void checkNextCommand();
        void pause();
        // Graceful stop - do not send new commands, wait for buffer to be empty and for idle state
        void abort();
        // Instant stop - reset device, clear queue, mark unackedcommands as aborted,
        void instantAbort();
};

#endif // RUNNINGBEHAVIOR_H
