// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "idlebehavior.h"
#include "runningbehavior.h"
#include "alarmbehavior.h"
#include "../communicator.h"

IdleBehavior::IdleBehavior(StateBehavior *previous, QObject *parent)
    : StateBehavior{previous, parent}
{}

void IdleBehavior::onDeviceStateChanged(DeviceState state)
{
    // Handle device state changes
    if (state == DeviceState::Run) {
        // Machine started running - transition to running behavior
        emit transition(this, new RunningBehavior(this));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior(this));
    }
}

void IdleBehavior::onCommandResponse(QString command, QStringList response)
{
    // Process command responses in idle state
    // This could be used to transition to other behaviors based on command responses
}