// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "initializationbehavior.h"
#include "runningbehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

InitializationBehavior::InitializationBehavior(QObject *parent) : StateBehavior{parent}
{

}

void InitializationBehavior::onDeviceStateChanged(DeviceState state)
{
    // Handle device state changes
    if (state == DeviceState::Run) {
        // Machine started running - transition to running behavior
        emit transition(this, new RunningBehavior(this));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

void InitializationBehavior::onCommandResponse(QString command, QStringList response)
{
    // Process command responses in idle state
    // This could be used to transition to other behaviors based on command responses
}

void InitializationBehavior::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Disconnected) {
        // Handle disconnection if necessary
        emit transition(this, new AlarmBehavior()); // Example: transition to alarm on disconnect
    }
}

void InitializationBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);
    communicator->openConnection();
}
