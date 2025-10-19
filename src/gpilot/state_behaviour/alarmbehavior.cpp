// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"

AlarmBehavior::AlarmBehavior(int alarmCode, QObject *parent)
    : StateBehavior{parent}
    , m_alarmCode(alarmCode)
{
}

void AlarmBehavior::onDeviceStateChanged(DeviceState state)
{
    qDebug() << "[AlarmBehavior] Device State Changed:" << static_cast<int>(state);
    // Handle device state changes
    if (state == DeviceState::Idle) {
        emit transition(this, new IdleBehavior());
    }
}

void AlarmBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[AlarmBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    setAlarmMessage();
    // Can send alarm state query if the controller supports it
    // m_communicator->sendCommand(CommandSource::System, "$?", TABLE_INDEX_UI);
}

void AlarmBehavior::setAlarmMessage()
{
    // Map alarm codes to meaningful user messages
    switch(m_alarmCode) {
        case 1:
            m_alarmMessage = "Hard limit triggered";
            break;
        case 2:
            m_alarmMessage = "G-code motion target exceeds machine travel";
            break;
        case 3:
            m_alarmMessage = "Reset while in motion";
            break;
        case 4:
            m_alarmMessage = "Probe fail";
            break;
        case 5:
            m_alarmMessage = "Probe fail: Initial probe not triggered";
            break;
        case 6:
            m_alarmMessage = "Homing fail: Could not find limit switch";
            break;
        case 7:
            m_alarmMessage = "Homing fail: Door open";
            break;
        case 8:
            m_alarmMessage = "Homing fail: Pull off travel failed";
            break;
        case 9:
            m_alarmMessage = "Homing fail: Could not find pull-off motion";
            break;
        default:
            m_alarmMessage = "Unknown alarm: " + QString::number(m_alarmCode);
    }
}

void AlarmBehavior::onCommandResponse(QString command, QString response, QStringList fullResponse)
{
    qDebug() << "[AlarmBehavior] Command Response:" << command << response;
    // Handle command responses in alarm state
    if (command == "$X") {  // Unlock command
        if (!response.contains("error")) {
            // Unlock successful, go to idle state
            // emit transition(this, new IdleBehavior(this));
        } else {
            // Unlock failed, stay in alarm state
            // emit error(this, "Failed to unlock alarm: " + response.join(" "));
        }
    }
}

// void AlarmBehavior::onConnectionStateChanged(ConnectionState state)
// {
//     qDebug() << "[AlarmBehavior] Connection State Changed:" << static_cast<int>(state);
//     if (state != ConnectionState::Connected) {
//         // If connection is lost, we might want to transition to a different state
//         // For now, we don't do anything special
//     }
// }

void AlarmBehavior::unlock()
{
    m_communicator->sendCommand(CommandSource::GeneralUI, "$X", TABLE_INDEX_UI);
}

bool AlarmBehavior::execute(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Unlock:
            unlock();
            return true;
    }
}

bool AlarmBehavior::isActionAllowed(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Unlock:
            return true;
    }

    return StateBehavior::isActionAllowed(action);
}
