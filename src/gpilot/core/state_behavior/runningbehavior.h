// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef RUNNINGBEHAVIOR_H
#define RUNNINGBEHAVIOR_H

#include <memory>
#include "abstractstatebehavior.h"
#include "core/gcode/gcode.h"
#include "core/communicator/commandscanner.h"

class RunningBehavior : public AbstractStateBehavior
{
    public:
        enum class Stage {
            Unknown,
            Running,
            Pausing,
            NoMoreCommands,
            // Idle has been observed but the command buffer still holds
            // unacknowledged commands. We wait for the remaining ok/error
            // responses before transitioning to IdleBehavior, so the program
            // table gets fully marked.
            Draining,
            Aborting,
        };

        explicit RunningBehavior(GCode &program, QObject *parent = nullptr);
        // Owning variant used for macros — the GCode lifetime matches this behavior's.
        explicit RunningBehavior(std::unique_ptr<GCode> ownedProgram, QObject *parent = nullptr);
        GCode &program() const { return m_program; }
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
        void onResponseProcessed() override;
        void onAlarm(int code) override;
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        // Running-specific methods
        void handleFeedOverride(int percentage);
        void handleSpindleOverride(int percentage);

    protected:
        QString name() const override { return "Running"; }
        bool doAction(const Action &action) override;
        void doOnMachineState(MachineState state) override;

    private:
        Stage m_stage;
        int m_feedOverride;
        int m_spindleOverride;
        std::unique_ptr<GCode> m_ownedProgram;
        GCode &m_program;
        CommandScanner m_commandScanner;
        int m_lastLookAheadIndex = -1;

        // Set when pause() was triggered by a failing command response.
        // onMachineStateChanged uses this to choose between PauseBehavior
        // (plain pause) and UserPromptBehavior (continue / abort prompt).
        QString m_errorCommand;
        QString m_errorDescription;
        int m_errorCode = 0;
        // Counts additional errors that arrived after the first one triggered
        // the pause — GRBL parses ahead of us, so by the time we react to the
        // first error there can be more already in flight. Surfaced in the
        // prompt so the user knows the full extent of the failure.
        int m_subsequentErrorCount = 0;
        void sendStreamerCommandsUntilBufferIsFull();
        void checkNextCommand();
        void pause();
        // Graceful stop - do not send new commands, wait for buffer to be empty and for idle state
        void abort();
        // Instant stop - reset device, clear queue, mark unackedcommands as aborted,
        void instantAbort();
};

#endif // RUNNINGBEHAVIOR_H
