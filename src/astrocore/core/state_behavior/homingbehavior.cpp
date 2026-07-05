// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "homingbehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"

HomingBehavior::HomingBehavior(QObject *parent)
    : AbstractStateBehavior{parent}
    , m_homingStarted(false)
    , m_homingCompleted(false)
{
}

AbstractStateBehavior::Result HomingBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Homing] Entry";

    // Send homing command
    m_communicator->sendCommand(CommandSource::GeneralUI, "$H", TABLE_INDEX_UI);
    m_homingStarted = true;
    m_homingCompleted = false;

    m_communicator->startQueryingMachineState();

    return Result::Ok;
}

void HomingBehavior::onMachineStateChanged(MachineState state)
{
    // Handle device state changes during homing process
    // if (state == DeviceState::Idle && m_homingStarted) {
    //     // Device went to idle state after homing started
    //     // This may mean the homing completed successfully
    //     m_homingCompleted = true;

    //     // Return to previous state or idle state
    //     if (m_previous) {
    //         emit resumePrevious();
    //     } else {
    //         emit transition(this, new IdleBehavior(this));
    //     }
    // } else if (state == DeviceState::Alarm && m_homingStarted) {
    //     // Alarm occurred during homing - homing likely failed
    //     emit error(this, "Homing failed - device entered alarm state");
    //     emit transition(this, new AlarmBehavior());
    // }
}

AbstractStateBehavior::Result HomingBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    qDebug() << "[Behavior][Homing] Command Response:" << command << response;
    if (command == "$H") {
        if (cmdStatus.ok) {
            qDebug() << "[Behavior][Homing] Homing command finished";
            // Got a response but it's not an error
            // Homing might be in progress or just finished
            // Some controllers don't give direct response about homing completion
            // So we check machine state in onMachineStateChanged

            // qDebug() << "[Behavior][Homing] Test";
            // qDebug() << "[Behavior][Homing] Test" << response;
            // qDebug() << "[Behavior][Homing] Test" << fullResponse;

            emit transition(this, new IdleBehavior(this));
        } else if (!response.isEmpty() && !m_homingCompleted) {
            qDebug() << "[Behavior][Homing] Homing error";
            // Error occurred during homing command
            emit error(this, "Homing failed - " + response);

            // Return to previous state or idle state
            if (m_previousType.has_value()) {
                emit resumePrevious();
            } else {
                emit transition(this, new IdleBehavior(this));
            }
        }

        return Result::Ok;
    }

    return Result::Unhandled;
}

void HomingBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][Homing] Alarm during homing:" << code;

    emit transition(this, new AlarmBehavior(code));
}

bool HomingBehavior::doAction(const Action &action)
{
    if (handleMachineConfigurationActions(action)) {
        return true;
    }

    return false;
}
