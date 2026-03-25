// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"
#include "toolchangebehavior.h"
#include "core/communicator/communicator.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

RunningBehavior::RunningBehavior(GCode &program, QObject *parent)
    : StateBehavior{parent}
    , m_feedOverride(100)
    , m_spindleOverride(100)
    , m_program(program)
{}

void RunningBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Idle) {
        if (m_stage == RunningStage::Running) {
            qWarning() << "[Behavior][Running] Unexpected transition to Idle state while running";
        } else if (m_stage == RunningStage::NoMoreCommands) {
            qDebug() << "[Behavior][Running][Dbg] Transition to Idle state after finishing commands, expected behavior";
        }

        emit transition(this, new IdleBehavior());
    } else if (m_stage != RunningStage::Resuming && (state == MachineState::Hold0 || state == MachineState::Hold1)) {
        PauseBehavior::PauseSource source = m_pause
            ? PauseBehavior::PauseSource::UserRequest
            : PauseBehavior::PauseSource::External;
        emit transition(this, new PauseBehavior(source));
    } else if (state == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior());
    }
}

StateBehavior::Result RunningBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(fullResponse);

    qDebug() << "[Behavior][Running][Resp] Response:" << command << "->" << response << "buffer length" << m_communicator->bufferLength();

    assert(commandAttributes.tableIndex >= 0);
    m_program.setCommandResponse(commandAttributes.tableIndex, response == "ok", enrichErrorMessage(response));

    static QRegularExpression m6("M0*6(?!\\d)");
    if (GcodePreprocessorUtils::removeComment(command).contains(m6)) {
        static QRegularExpression toolNumber("T(\\d+)");
        QRegularExpressionMatch match = toolNumber.match(command);
        int tool = match.hasMatch() ? match.captured(1).toInt() : 0;
        emit transition(this, new ToolChangeBehavior(tool));

        return Result::Ok;
    }

    if (!m_pause) {
        sendStreamerCommandsUntilBufferIsFull();
    }

    return Result::Ok;;
}

void RunningBehavior::onAlarm(int code)
{
    // Handle alarm during running state
    emit transition(this, new AlarmBehavior(code));
}

bool RunningBehavior::doAction(const Action &action)
{
    if (action.type() == Action::Type::PauseResume && !m_pause) {
        pause();

        return true;
    } else if (action.type() == Action::Type::Stop) {
        m_communicator->clearCommandsAndQueue();
        m_communicator->sendRealtimeCommand(GRBL_LIVE_SOFT_RESET);

        return true;
    }

    return StateBehavior::doAction(action);
}

StateBehavior::Result RunningBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[Behavior][Running] Entry";
    StateBehavior::onEntry(communicator, previous);

    m_pause = false;
    communicator->startQueryingMachineState();

    PauseBehavior* pausePrevious = dynamic_cast<PauseBehavior*>(previous);
    if (pausePrevious) {
        qDebug() << "[Behavior][Running] Resuming from Pause, sending Cycle Start and waiting for Run state";

        // Machine is in Hold — send Cycle Start and wait for Run state before filling buffer
        m_stage = RunningStage::Resuming;
        m_communicator->sendRealtimeCommand(GRBL_LIVE_CYCLE_START);
        waitForStateResponse([this](MachineState state) {
            switch (state) {
                case MachineState::Run:
                    qDebug() << "[Behavior][Running] Detected Run state";
                    sendStreamerCommandsUntilBufferIsFull();
                    break;
                case MachineState::Unknown:
                    qDebug() << "[Behavior][Running] Timeout waiting for Run state";
                    break;
            }
            m_stage = RunningStage::Unknown;
        }, MachineState::Run, 500);
    } else {
        m_stage = RunningStage::Running;
    }

    sendStreamerCommandsUntilBufferIsFull();

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

        m_stage = RunningStage::NoMoreCommands;

        return;
    }

    // Pass empty commands through loop too, we will skip them inside
    QString command = m_program.command();
    int sent = 0;
    while (command.isEmpty() || (
        !m_communicator->willOverflowBuffer(command) && m_program.hasMoreCommands()
        && !(!m_communicator->isCommandBufferEmpty() && GcodePreprocessorUtils::removeComment(m_communicator->commands().last().commandLine).contains(M230))
    )) {
        if (command.isEmpty()) {
            m_program.setCommandSkipped();
        } else {
            m_program.setCommandSent();
            m_communicator->sendCommand(CommandSource::Program, command, m_program.commandIndex());
            // qDebug() << "[Behavior][Running] Sent command:" << command;
            sent++;
        }
        if (!m_program.isLastCommand()) {
            m_program.advanceCommandIndex();

            command = m_program.command();
        } else {
            break;
        }
    }

    qDebug() << "[Behavior][Running][Dbg] Sent " << sent << "; buffer length after commands sent" << m_communicator->bufferLength();
}

void RunningBehavior::pause()
{
    if (m_pause) {
        qDebug() << "[Behavior][Running] Already paused, ignoring pause request";

        return;
    }

    qDebug() << "[Behavior][Running] Pausing";
    m_pause = true;
    m_communicator->sendRealtimeCommand(GRBL_LIVE_FEED_HOLD);
}

