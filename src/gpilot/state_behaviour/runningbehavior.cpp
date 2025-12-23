// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"
// #include "toolchangebehavior.h"
#include "core/communicator/communicator.h"
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
        // Program finished or was stopped
        // emit transition(this, new IdleBehavior());
    } else if (m_pause && (state == MachineState::Hold0 || state == MachineState::Hold1)) {
        // Machine is in hold state - transition to pause
        emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::Program));
    } else if (state == MachineState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

StateBehavior::Result RunningBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(fullResponse);

    qDebug() << "[RunningBehavior] onCommandResponse:" << command << "->" << response << "buffer length" << m_communicator->bufferLength();

    m_program.setCommandResponse(commandAttributes.tableIndex, response == "ok", enrichErrorMessage(response));

    // Process command responses during running state
    // For example, handle M6 commands for tool change
    // if (command.contains("M6")) {
    //     // Tool change requested
    //     // emit transition(this, new ToolChangeBehavior(this, command.mid(command.indexOf("T") + 1).toInt(),
    //     //                                           ToolChangeBehavior::ToolChangeSource::Program));

    //     return true;
    // }

    if (!m_pause) {
        sendStreamerCommandsUntilBufferIsFull();
    }

    return Result::Ok;;
}

void RunningBehavior::onAlarm(int code)
{
    if (m_pause) {

    }

    // Handle alarm during running state
    emit transition(this, new AlarmBehavior(code));
}

bool RunningBehavior::doAction(const Action &action)
{
    if (action.type() == Action::Type::Pause && !m_pause) {
        pause();
        // Handle pause action
        // emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::UserRequest));
        return true;
    } else if (action.type() == Action::Type::Stop) {
        // Handle stop action
        // Send stop command to controller
        if (m_communicator) {
            // m_communicator->sendCommand(CommandSource::User, "M0");
        }

        return true;
    }

    return StateBehavior::doAction(action);
}

StateBehavior::Result RunningBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[RunningBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    // If coming from PauseBehavior, resume the program
    PauseBehavior* pausePrevious  = dynamic_cast<PauseBehavior*>(previous);
    if (pausePrevious) {
        resume();
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

    // qDebug() <<
    //     "bufferLength: " << m_communicator->bufferLength() <<
    //     "command.length: " << command.length() <<
    //     "commandIndex: " << m_program.commandIndex() <<
    //     "hasMoreCommands: " << m_program.hasMoreCommands() <<
    //     "m_commands.isEmpty: " << (!m_communicator->m_commands.isEmpty() && GcodePreprocessorUtils::removeComment(m_commands.last().commandLine).contains(M230));

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
            // qDebug() << "[RunningBehavior] Sent command:" << command;
            sent++;
        }
        if (!m_program.isLastCommand()) {
            m_program.advanceCommandIndex();

            command = m_program.command();
        } else {
            break;
        }
    }

    qDebug() << "[RunningBehavior] Sent " << sent << "; buffer length after commands sent" << m_communicator->bufferLength();
}

void RunningBehavior::pause()
{
    qDebug() << "[RunningBehavior] Pausing";
    m_pause = true;
    // m_communicator->sendRealtimeCommand(GRBL_LIVE_FEED_HOLD);
}

void RunningBehavior::resume()
{
    qDebug() << "[RunningBehavior] Resuming";
    // m_communicator->sendRealtimeCommand(GRBL_LIVE_CYCLE_START);
    m_pause = false;
}
