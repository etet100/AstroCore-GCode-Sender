// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
// #include "pausebehavior.h"
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
        emit transition(this, new IdleBehavior());
    } else if (state == MachineState::Hold0 || state == MachineState::Hold1) {
        // Machine is in hold state - transition to pause
        // emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::Program));
    } else if (state == MachineState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

bool RunningBehavior::onCommandResponse(QString command, QString response, QStringList fullResponse)
{
    qDebug() << "[RunningBehavior] onCommandResponse:" << command << "->" << response;

    // Process command responses during running state
    // For example, handle M6 commands for tool change
    // if (command.contains("M6")) {
    //     // Tool change requested
    //     // emit transition(this, new ToolChangeBehavior(this, command.mid(command.indexOf("T") + 1).toInt(),
    //     //                                           ToolChangeBehavior::ToolChangeSource::Program));

    //     return true;
    // }

    sendStreamerCommandsUntilBufferIsFull();

    return true;
}

void RunningBehavior::onAlarm(int code)
{
    // Handle alarm during running state
    emit transition(this, new AlarmBehavior(code));
}

StateBehavior::Result RunningBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[RunningBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

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
    if (m_communicator->m_queue.length() > 0) return;

    static QRegularExpression M230("(M0*2|M30|M0*6)(?!\\d)");

    // qDebug() <<
    //     "bufferLength: " << m_communicator->bufferLength() <<
    //     "command.length: " << command.length() <<
    //     "commandIndex: " << m_program.commandIndex() <<
    //     "hasMoreCommands: " << m_program.hasMoreCommands() <<
    //     "m_commands.isEmpty: " << (!m_communicator->m_commands.isEmpty() && GcodePreprocessorUtils::removeComment(m_commands.last().commandLine).contains(M230));

    // Pass empty commands through loop too, we will skip them inside
    QString command = m_program.command();
    while (command.isEmpty() || (
        !m_communicator->willOverflowBuffer(command) && m_program.hasMoreCommands()
        && !(!m_communicator->m_commands.isEmpty() && GcodePreprocessorUtils::removeComment(m_communicator->m_commands.last().commandLine).contains(M230))
    )) {
        if (command.isEmpty()) {
            m_program.commandSkipped();
        } else {
            m_program.commandSent();
            m_communicator->sendCommand(CommandSource::Program, command, m_program.commandIndex());
        }
        m_program.advanceCommandIndex();
        command = m_program.command();
    }
}
