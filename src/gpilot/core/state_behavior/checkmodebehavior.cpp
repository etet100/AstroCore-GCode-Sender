// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "checkmodebehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

CheckModeBehavior::CheckModeBehavior(GCode &program, QObject *parent)
    : StateBehavior{parent}
    , m_program(program)
    , m_stopped(false)
{
}

QString CheckModeBehavior::description()
{
    return "Check Mode (Dry Run)";
}

StateBehavior::Result CheckModeBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    qDebug() << "[Behavior][CheckMode] Entering check mode. Program will be verified without actual movement.";

    // Enter check mode by sending $C command
    m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);

    // Start sending G-code commands for verification
    sendStreamerCommandsUntilBufferIsFull();

    return Result::Ok;
}

StateBehavior::Result CheckModeBehavior::onExit(StateBehavior *next)
{
    Q_UNUSED(next);

    // Exit check mode by sending $C command again (toggle)
    if (m_communicator && m_communicator->machineState() == MachineState::Check) {
        m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
    }

    return StateBehavior::onExit(next);
}

void CheckModeBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Idle) {
        // Check mode finished or was stopped
        qDebug() << "[Behavior][CheckMode] Check mode completed. Returning to Idle.";
        emit transition(this, new IdleBehavior());
    } else if (state == MachineState::Alarm) {
        // Machine entered alarm state during check
        emit transition(this, new AlarmBehavior());
    }
}

StateBehavior::Result CheckModeBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(fullResponse);

    qDebug() << "[Behavior][CheckMode] Command:" << command << "Response:" << response;

    m_program.setCommandResponse(commandAttributes.tableIndex, response == "ok", enrichErrorMessage(response));

    // Continue sending commands if not stopped
    if (!m_stopped) {
        sendStreamerCommandsUntilBufferIsFull();
    }

    return Result::Ok;
}

void CheckModeBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][CheckMode] Alarm during check mode:" << code;
    emit transition(this, new AlarmBehavior(code));
}

bool CheckModeBehavior::doAction(const Action &action)
{
    if (action.type() == Action::Type::Stop) {
        stop();
        return true;
    }

    return StateBehavior::doAction(action);
}

void CheckModeBehavior::sendStreamerCommandsUntilBufferIsFull()
{
    if (!m_communicator || !m_communicator->isQueueEmpty()) {
        return;
    }

    // Pass empty commands through loop too, we will skip them inside
    QString command = m_program.command();
    int sent = 0;

    while (command.isEmpty() || (!m_communicator->willOverflowBuffer(command) && m_program.hasMoreCommands())) {
        if (command.isEmpty()) {
            m_program.setCommandSkipped();
        } else {
            m_program.setCommandSent();
            m_communicator->sendCommand(CommandSource::Program, command, m_program.commandIndex());
            sent++;
        }

        if (!m_program.isLastCommand()) {
            m_program.advanceCommandIndex();
            command = m_program.command();
        } else {
            break;
        }
    }

    qDebug() << "[Behavior][CheckMode] Sent " << sent << " commands in check mode; buffer length:" << m_communicator->bufferLength();
}

void CheckModeBehavior::stop()
{
    m_stopped = true;
    qDebug() << "[Behavior][CheckMode] Check mode stopped by user.";

    // Clear remaining commands and exit check mode
    if (m_communicator) {
        m_communicator->clearQueue();
        m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
    }

    emit transition(this, new IdleBehavior());
}
