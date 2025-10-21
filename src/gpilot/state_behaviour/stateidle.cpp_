// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "stateidle.h"
#include "staterunning.h"
#include "statealarm.h"
#include "../communicator.h"

StateIdle::StateIdle(State *previous, QObject *parent)
    : State{previous, parent}
{}

void StateIdle::onDeviceStateChanged(DeviceState state)
{
    // Handle device state changes
    if (state == DeviceState::Run) {
        // Machine started running - transition to running state
        emit transition(this, new StateRunning(this));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new StateAlarm(this));
    }
}

void StateIdle::onCommandResponse(QString command, QStringList response)
{
    // Process command responses in idle state
    // This could be used to transition to other states based on command responses
}
