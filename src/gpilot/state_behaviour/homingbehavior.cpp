// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "homingbehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"

HomingBehavior::HomingBehavior(QObject *parent)
    : StateBehavior{parent}
    , m_homingStarted(false)
    , m_homingCompleted(false)
{
}

void HomingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[HomingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    // Send homing command
    m_communicator->sendCommand(CommandSource::GeneralUI, "$H", TABLE_INDEX_UI);
    m_homingStarted = true;
    m_homingCompleted = false;
}

void HomingBehavior::onDeviceStateChanged(DeviceState state)
{
    // Handle device state changes during homing process
    // if (state == DeviceState::Idle && m_homingStarted) {
    //     // Device went to idle state after homing started
    //     // This may mean the homing completed successfully
    //     m_homingCompleted = true;

    //     // Return to previous state or idle state
    //     if (m_previous) {
    //         emit transition(this, m_previous);
    //     } else {
    //         emit transition(this, new IdleBehavior(this));
    //     }
    // } else if (state == DeviceState::Alarm && m_homingStarted) {
    //     // Alarm occurred during homing - homing likely failed
    //     emit error(this, "Homing failed - device entered alarm state");
    //     emit transition(this, new AlarmBehavior());
    // }
}

void HomingBehavior::onCommandResponse(QString command, QStringList response)
{
    qDebug() << "[HomingBehavior] Command Response:" << command << response;
    if (command == "$H") {
        if (response.contains("error")) {
            qDebug() << "[HomingBehavior] Homing error";
            // Error occurred during homing command
            emit error(this, "Homing failed - " + response.join(" "));

            // Return to previous state or idle state
            if (m_previous) {
                emit transition(this, m_previous);
            } else {
                emit transition(this, new IdleBehavior(this));
            }
        } else if (!response.isEmpty() && !m_homingCompleted) {
            qDebug() << "[HomingBehavior] Homing command finished";
            // Got a response but it's not an error
            // Homing might be in progress or just finished
            // Some controllers don't give direct response about homing completion
            // So we check device state in onDeviceStateChanged

            emit transition(this, new IdleBehavior(this));
        }
    }
}
