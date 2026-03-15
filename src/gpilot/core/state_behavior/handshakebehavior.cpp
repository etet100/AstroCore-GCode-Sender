// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "handshakebehavior.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

HandshakeBehavior::HandshakeBehavior(QObject *parent)
    : StateBehavior{parent}
{}

QString HandshakeBehavior::description() { return "Handshake"; }

StateBehavior::Result HandshakeBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[Behavior][Handshake] Entry — querying machine state.";
    StateBehavior::onEntry(communicator, previous);

    m_communicator->startQueryingMachineState();

    return Result::Ok;
}

StateBehavior::Result HandshakeBehavior::onExit(StateBehavior *next)
{
    qDebug() << "[Behavior][Handshake] Exit.";

    return StateBehavior::onExit(next);
}

void HandshakeBehavior::onMachineState(MachineState state)
{
    if (m_stage != QueryingState) {
        return;
    }

    qDebug() << "[Behavior][Handshake] Initial machine state:" << static_cast<int>(state);
    m_initialState = state;

    // Machine is actively running something we didn't start — skip settings query.
    if (state == MachineState::Run || state == MachineState::Jog || state == MachineState::Check) {
        log("[Handshake] Machine is busy with an external process. Skipping settings query.");
        emit transition(this, new ExternalProcessBehavior());

        return;
    }

    log("[Handshake] Querying device settings.");
    m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    m_stage = QueryingSettings;
}

StateBehavior::Result HandshakeBehavior::onCommandResponse(
    QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus,
    QString response, QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);
    Q_UNUSED(response);

    qDebug() << "[Behavior][Handshake] Command response:" << command;

    // Retry on EEPROM-not-ready error before main handling.
    if (!cmdStatus.ok && cmdStatus.errorCode == GRBL_ERROR_EEPROM_READ_FAIL) {
        if (command == "$$" || command == "$#") {
            qDebug() << "[Behavior][Handshake] EEPROM not ready for" << command << "— requeuing.";

            return Result::ReturnCommandToQueue;
        }
    }

    if (command == "$$") {
        if (cmdStatus.ok) {
            m_communicator->processDeviceConfiguration(fullResponse);
        } else {
            qDebug() << "[Behavior][Handshake] Failed to receive device settings (error" << cmdStatus.errorCode << "). Continuing.";
        }

        m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1);
        m_stage = QueryingOffsets;

        return Result::Ok;
    }

    if (command == "$#") {
        if (cmdStatus.ok) {
            m_communicator->processOffsetsVars(fullResponse);
        } else {
            qDebug() << "[Behavior][Handshake] Failed to receive coordinate offsets (error" << cmdStatus.errorCode << "). Continuing.";
        }

        m_stage = Completed;
        transitionBasedOnInitialState();

        return Result::Ok;
    }

    return Result::Unhandled;
}

void HandshakeBehavior::transitionBasedOnInitialState()
{
    qDebug() << "[Behavior][Handshake] Handshake complete. Transitioning based on state:" << static_cast<int>(m_initialState);

    switch (m_initialState) {
        case MachineState::Alarm:
            log("[Handshake] Machine is in alarm state.");
            emit transition(this, new AlarmBehavior(m_communicator->lastAlarmCode()));
            break;

        case MachineState::Run:
        case MachineState::Jog:
        case MachineState::Check:
        case MachineState::Hold0:
        case MachineState::Hold1:
        case MachineState::Door0:
        case MachineState::Door1:
        case MachineState::Door2:
        case MachineState::Door3:
            log("[Handshake] Machine is in an active/held state — treating as external process.");
            emit transition(this, new ExternalProcessBehavior());
            break;

        default:
            // Idle, Home, Sleep, Unknown — treat as idle.
            emit transition(this, new IdleBehavior());
            break;
    }
}
