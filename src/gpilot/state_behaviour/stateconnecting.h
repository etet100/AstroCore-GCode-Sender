// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATECONNECTING_H
#define STATECONNECTING_H

#include "state.h"

class StateConnecting : public State
{
    public:
        StateConnecting(State *previous, QObject *parent = nullptr);
        QString name() override { return "Connecting"; }
        void onConnectionStateChanged(ConnectionState state) override;
};

#endif // STATECONNECTING_H
