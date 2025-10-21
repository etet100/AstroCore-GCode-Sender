// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "stateconnecting.h"
#include "stateidle.h"

StateConnecting::StateConnecting(State *previous, QObject *parent): State{previous, parent}
{
}

void StateConnecting::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Connected)
    {
        emit transition(this, new StateIdle(this));
    }
}
