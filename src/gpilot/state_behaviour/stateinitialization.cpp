// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "stateinitialization.h"
#include "stateconnecting.h"

StateInitialization::StateInitialization(State *previous, QObject *parent)
    : State{previous, parent}
{}

void StateInitialization::onDeviceStateChanged(DeviceState state)
{
    qDebug() << "StateInitialization::onDeviceStateChanged" << (int) state;
}

void StateInitialization::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Connecting) {
        transition(m_previous, new StateConnecting(this));
    }
}

void StateInitialization::onEntry(Communicator *communicator, State *previous)
{
    State::onEntry(communicator, previous);
}
