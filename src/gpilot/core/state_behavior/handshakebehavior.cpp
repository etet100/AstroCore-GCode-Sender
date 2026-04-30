// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "handshakebehavior.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

HandshakeBehavior::HandshakeBehavior(QObject *parent)
    : AbstractStateBehavior{parent}
{}

QString HandshakeBehavior::description() { return "Handshake"; }

AbstractStateBehavior::Result HandshakeBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Handshake] Entry — querying machine state.";

    m_communicator->startQueryingMachineState();

    return Result::Ok;
}

AbstractStateBehavior::Result HandshakeBehavior::doOnExit(AbstractStateBehavior *next)
{
    qDebug() << "[Behavior][Handshake] Exit.";

    return Result::Ok;
}

void HandshakeBehavior::doOnMachineState(MachineState state)
{
    if (m_stage != QueryingState) {
        return;
    }

    qDebug() << "[Behavior][Handshake] Initial machine state:" << static_cast<int>(state);
    m_initialState = state;

    // Machine is actively running something we didn't start — skip settings query.
    if (
        state != MachineState::Idle &&
        state != MachineState::Alarm &&
        state != MachineState::Check
    ) {
        qDebug() << "[Behavior][Handshake] Machine is busy with an external process. Skipping settings query.";
        log("[Handshake] Machine is busy with an external process. Skipping settings query.");
        emit transition(this, new ExternalProcessBehavior());

        return;
    }

    log("[Handshake] Querying device settings.");
    m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    m_stage = QueryingSettings;
}

AbstractStateBehavior::Result HandshakeBehavior::onCommandResponse(
    QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus,
    QString response, QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);
    Q_UNUSED(response);

    qDebug() << "[Behavior][Handshake] Command response:" << command;

    // Retry on EEPROM-not-ready error before main handling.
    if (!cmdStatus.ok && cmdStatus.errorCode == GRBL_ERROR_EEPROM_READ_FAIL) {
        if (command == "$$" || command == "$G" || command == "$#") {
            qDebug() << "[Behavior][Handshake] EEPROM not ready for" << command << "— requeuing.";

            return Result::ReturnCommandToQueue;
        }
    }

    // Responses for all commands are handled in Communicator callback, do not handle it here.

    if (command == "$$") {
        if (!cmdStatus.ok) {
            qDebug() << "[Behavior][Handshake] Failed to receive device settings (error" << cmdStatus.errorCode << "). Continuing.";
        }

        m_communicator->sendCommand(CommandSource::System, "$G", TABLE_INDEX_UTIL1);
        m_stage = QueryingGcodeState;

        return Result::Ok;
    }

    if (command == "$G") {
        if (!cmdStatus.ok) {
            qDebug() << "[Behavior][Handshake] Failed to receive gcode parser state (error" << cmdStatus.errorCode << "). Continuing.";
        }

        m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1);
        m_stage = QueryingOffsets;

        return Result::Ok;
    }

    if (command == "$#") {
        if (!cmdStatus.ok) {
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

        case MachineState::Check:
            log("[Handshake] Machine is in check mode.");
            emit transition(this, new CheckModeBehavior());
            break;

        case MachineState::Run:
        case MachineState::Jog:
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
