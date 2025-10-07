// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statealarm.h"
#include "../communicator.h"
#include "stateidle.h"

StateAlarm::StateAlarm(State *previous, int alarmCode, QObject *parent)
    : State{previous, parent}
    , m_alarmCode(alarmCode)
{
    setAlarmMessage();
}

void StateAlarm::onEntry(Communicator *communicator, State *previous)
{
    State::onEntry(communicator, previous);

    // Possibly query for additional information about the alarm
    // m_communicator->sendCommand(CommandSource::System, "$G", TABLE_INDEX_UI);
}

void StateAlarm::setAlarmMessage()
{
    // Map alarm codes to meaningful messages for the user
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

void StateAlarm::onCommandResponse(QString command, QStringList response)
{
    // Handle responses to commands sent during alarm state
    if (command == "$X") {  // Unlock command
        if (!response.contains("error")) {
            emit transition(this, new StateIdle(this));
        }
    }
}

void StateAlarm::onConnectionStateChanged(ConnectionState state)
{
    if (state != ConnectionState::Connected) {
        // If connection is lost, we might want to transition to another state
    }
}