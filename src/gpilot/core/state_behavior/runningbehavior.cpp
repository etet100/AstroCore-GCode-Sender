// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/core.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"
#include "toolchangebehavior.h"
#include "userpromptbehavior.h"
#include "core/communicator/communicator.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

RunningBehavior::RunningBehavior(GCode &program, QObject *parent)
    : AbstractStateBehavior{parent}
    , m_feedOverride(100)
    , m_spindleOverride(100)
    , m_program(program)
{}

void RunningBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Idle) {
        if (m_stage == Stage::Aborting) {
            qDebug() << "[Behavior][Running] Abort completed, transitioning to Idle";
        } else if (m_stage == Stage::Running) {
            qWarning() << "[Behavior][Running] Unexpected transition to Idle state while running";
        } else if (m_stage == Stage::NoMoreCommands) {
            qDebug() << "[Behavior][Running] Program finished, transitioning to Idle";
        }

        Core::instance().timer().stopExecution();
        emit transition(this, new IdleBehavior());
    } else if (m_stage == Stage::Pausing && (state == MachineState::Hold0 || state == MachineState::Hold1)) {
        // Error-driven pause: ask the user whether to continue past the bad
        // command or abort the program. Plain pauses fall through to PauseBehavior.
        if (!m_errorDescription.isEmpty()) {
            PromptSpec spec;
            spec.promptId = "running.command-error";
            spec.title = "Command error";
            spec.message = QString("Command \"%1\" failed: %2.")
                               .arg(m_errorCommand, m_errorDescription);
            spec.context = {
                {"command", m_errorCommand},
                {"errorCode", m_errorCode},
                {"reason", m_errorDescription},
            };
            spec.choices = {
                {"continue", "Continue", /*destructive=*/false, /*isDefault=*/false},
                {"abort", "Abort", /*destructive=*/true, /*isDefault=*/true},
            };
            emit transition(this, new UserPromptBehavior(spec), TransitionKind::Suspend);

            return;
        }

        PauseBehavior::PauseSource source = m_stage == Stage::Pausing
            ? PauseBehavior::PauseSource::UserRequest
            : PauseBehavior::PauseSource::External;
        emit transition(this, new PauseBehavior(source), TransitionKind::Suspend);
    } else if (state == MachineState::Alarm) {
        Core::instance().timer().stopExecution();
        emit transition(this, new AlarmBehavior());
    }
}

AbstractStateBehavior::Result RunningBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(fullResponse);

    qDebug() << "[Behavior][Running][Resp] Response:" << command << "->" << response << "buffer length" << m_communicator->bufferLength();

    if (commandAttributes.tableIndex < 0) {
        qDebug() << "[Behavior][Running][Resp] Ignoring response for non-program command:" << command;

        return Result::Ok;
    }

    assert(commandAttributes.tableIndex >= 0);
    m_program.setCommandResponse(commandAttributes.tableIndex, cmdStatus.ok, enrichErrorMessage(response));

    if (!cmdStatus.ok && m_stage == Stage::Running) {
        if (m_configuration->senderModule().ignoreErrorResponses()) {
            qWarning() << "[Behavior][Running][Resp] Command error" << cmdStatus.errorCode
                       << "for" << command << "— ignoring (ignoreErrorResponses is set)";
            sendStreamerCommandsUntilBufferIsFull();

            return Result::Ok;
        }

        qWarning() << "[Behavior][Running][Resp] Command error" << cmdStatus.errorCode
                   << "for" << command << "— pausing program";
        m_errorCommand = command;
        m_errorDescription = enrichErrorMessage(response);
        m_errorCode = cmdStatus.errorCode;
        pause();

        return Result::Ok;
    }

    static QRegularExpression m6("M0*6(?!\\d)");
    if (GcodePreprocessorUtils::removeComment(command).contains(m6)) {
        static QRegularExpression toolNumber("T(\\d+)");
        QRegularExpressionMatch match = toolNumber.match(command);
        int tool = match.hasMatch() ? match.captured(1).toInt() : 0;
        emit transition(this, new ToolChangeBehavior(tool), TransitionKind::Suspend);

        return Result::Ok;
    }

    if (m_stage == Stage::Running) {
        sendStreamerCommandsUntilBufferIsFull();
    } else {
        qDebug() << "[Behavior][Running][Dbg] Not sending next commands, current stage is" << static_cast<int>(m_stage);
    }

    return Result::Ok;
}

void RunningBehavior::onAlarm(int code)
{
    Core::instance().timer().stopExecution();
    emit transition(this, new AlarmBehavior(code));
}

bool RunningBehavior::doAction(const Action &action)
{
    if (action.type() == Action::Type::Pause && m_stage == Stage::Running) {
        pause();

        return true;
    } else if (action.type() == Action::Type::Abort) {
        abort();

        return true;
    }

    return AbstractStateBehavior::doAction(action);
}

AbstractStateBehavior::Result RunningBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Running] Entry";

    if (!m_program.hasMoreCommands()) {
        qDebug() << "[Behavior][Running] No commands to run on entry, transitioning to Idle";
        emit transition(this, new IdleBehavior());

        return Result::Ok;
    }

    communicator->startQueryingMachineState();

    // Resumed either from a plain Pause or from a UserPrompt (the latter is
    // raised by an error-driven pause). Both paths normalize to a single
    // "abort" / "resume" decision read from exit data.
    const bool resumingFromPause = (ctx.previousType == Type::Pause);
    const bool resumingFromPrompt = (ctx.previousType == Type::UserPrompt);
    if (resumingFromPause || resumingFromPrompt) {
        const QString decision = resumingFromPrompt
            ? ctx.data.value("choiceId").toString()           // "continue" | "abort"
            : ctx.data.value("action").toString();            // "resume"   | "abort"

        const bool abort = (decision == "abort");

        qDebug() << "[Behavior][Running] Resuming from"
                 << (resumingFromPrompt ? "UserPrompt" : "Pause")
                 << "decision:" << decision;

        if (abort) {
            qDebug() << "[Behavior][Running] Previous behavior requested abort";
            this->abort();

            return Result::Ok;
        } else {
            // Past the error or the pause — clear any error context.
            m_errorCommand.clear();
            m_errorDescription.clear();
            m_errorCode = 0;

            Core::instance().timer().resumeExecution();

            // Machine is in Hold — send Cycle Start and wait for Run state before filling buffer
            m_stage = Stage::Resuming;
            m_communicator->sendRealtimeCommand(GRBL_LIVE_CYCLE_START);
            waitForStateResponse([this](MachineState state) {
                switch (state) {
                    case MachineState::Run:
                        qDebug() << "[Behavior][Running] Detected Run state";
                        m_stage = Stage::Running;
                        sendStreamerCommandsUntilBufferIsFull();
                        break;

                    case MachineState::Unknown:
                        qDebug() << "[Behavior][Running] Timeout waiting for Run state";
                        m_stage = Stage::Unknown;
                        break;
                }
            }, MachineState::Run, 500);
        }
    } else {
        Core::instance().timer().startExecution();
        m_stage = Stage::Running;
    }

    sendStreamerCommandsUntilBufferIsFull();

    // Nothing got sent or queued — the machine will not move and the usual
    // Run -> Idle transition in onMachineStateChanged will never fire.
    if (m_stage != Stage::Resuming
        && m_communicator->bufferLength() == 0
        && m_communicator->isQueueEmpty()) {
        qDebug() << "[Behavior][Running] Nothing queued on entry, transitioning to Idle";
        emit transition(this, new IdleBehavior());
    }

    return Result::Ok;
}

void RunningBehavior::handleFeedOverride(int percentage)
{
    // Validate range
    if (percentage < 10) percentage = 10;
    if (percentage > 200) percentage = 200;

    m_feedOverride = percentage;

    // Send real-time override command to the controller
    // Assuming we have methods to send real-time override commands
    // for GRBL controllers
    // if (m_communicator) {
    //     // Code to send override commands depending on controller type
    //     // m_communicator->sendFeedOverride(percentage);
    // }
}

void RunningBehavior::handleSpindleOverride(int percentage)
{
    // Validate range
    if (percentage < 10) percentage = 10;
    if (percentage > 200) percentage = 200;

    m_spindleOverride = percentage;

    // Send real-time override command to the controller
    if (m_communicator.isNull()) {
        // Code to send spindle override commands
        // m_communicator->sendSpindleOverride(percentage);
    }
}

void RunningBehavior::sendStreamerCommandsUntilBufferIsFull()
{
    if (!m_communicator->isQueueEmpty()) {
        return;
    }

    static QRegularExpression M230("(M0*2|M30|M0*6)(?!\\d)");

    qDebug() << "[Behavior][Running][Dbg] " <<
        "bufferLength: " << m_communicator->bufferLength() <<
        "commandIndex: " << m_program.commandIndex() <<
        "hasMoreCommands: " << m_program.hasMoreCommands();

    if (!m_program.hasMoreCommands()) {
        qDebug() << "[Behavior][Running] No more commands to send";

        m_stage = Stage::NoMoreCommands;

        return;
    }

    // Pass empty commands through loop too, we will skip them inside
    QString command = m_program.command();
    int sent = 0;
    while (command.isEmpty() || (
        !m_communicator->willOverflowBuffer(command) && m_program.hasMoreCommands()
        && !(!m_communicator->isCommandBufferEmpty() && GcodePreprocessorUtils::removeComment(m_communicator->commands().last().commandLine).contains(M230))
    )) {
        checkNextCommand();
        if (command.isEmpty()) {
            m_program.setCommandSkipped();
        } else {
            m_program.setCommandSent();
            m_communicator->sendCommand(CommandSource::Program, command, m_program.commandIndex());
            // qDebug() << "[Behavior][Running] Sent command:" << command;
            sent++;
        }
        if (m_program.isLastCommand()) {
            // Advance past the end so hasMoreCommands() flips to false — otherwise the
            // next sendStreamer call (e.g. from onCommandResponse) would resend the
            // last command in a loop.
            m_program.advanceCommandIndex();
            break;
        }

        m_program.advanceCommandIndex();
        command = m_program.command();
    }

    if (!m_program.hasMoreCommands()) {
        m_stage = Stage::NoMoreCommands;
    }

    qDebug() << "[Behavior][Running][Dbg] Sent " << sent << "; buffer length after commands sent" << m_communicator->bufferLength();
}

void RunningBehavior::checkNextCommand()
{
    const int currentIndex = m_program.commandIndex();
    if (currentIndex == m_lastLookAheadIndex) {
        return;
    }
    m_lastLookAheadIndex = currentIndex;

    const GCodeItem *next = m_program.lookAhead(currentIndex, 1);
    if (!next) {
        qDebug() << "[Behavior][Running] Look-ahead: no next command after index" << currentIndex;

        return;
    }

    const QString nextCommand = next->command();
    const CommandScanner::CommandType type = m_commandScanner.classify(nextCommand);
    if (type == CommandScanner::CommandType::Pause) {
        qDebug() << "[Behavior][Running] Look-ahead: next command is Pause:" << nextCommand << "at index" << (currentIndex + 1);
    } else if (type == CommandScanner::CommandType::ToolChange) {
        qDebug() << "[Behavior][Running] Look-ahead: next command is ToolChange:" << nextCommand << "at index" << (currentIndex + 1);
    }
}

void RunningBehavior::pause()
{
    if (m_stage == Stage::Pausing) {
        qDebug() << "[Behavior][Running] Already pausing, ignoring pause request";

        return;
    }

    qDebug() << "[Behavior][Running] Pausing";
    m_stage = Stage::Pausing;
    m_communicator->sendRealtimeCommand(GRBL_LIVE_FEED_HOLD);
}

void RunningBehavior::instantAbort()
{
    qDebug() << "[Behavior][Running] Aborting — clearing queue, waiting for idle or alarm state";

    m_stage = Stage::Aborting;

    // Mark all non-acked commands as aborted
    for (auto &cmd : m_communicator->commandBuffer()->commands()) {
        // Ignore non-program commands
        if (cmd.tableIndex >= 0) {
            m_program.setCommandAborted(cmd.tableIndex);
        }
    }
    m_communicator->clearCommandsAndQueue();
    m_communicator->sendRealtimeCommand(GRBL_LIVE_SOFT_RESET);
}

void RunningBehavior::abort()
{
    qDebug() << "[Behavior][Running] Aborting — waiting for idle or alarm state";

    m_stage = Stage::Aborting;
}
