// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEINITIALIZATION_H
#define STATEINITIALIZATION_H

#include "state.h"

class StateInitialization : public State
{
    public:
        explicit StateInitialization(State *previous, QObject *parent = nullptr);
        QString name() override { return "Initialization"; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return false; }
        void onDeviceStateChanged(DeviceState state) override;
        void onConnectionStateChanged(ConnectionState state) override;
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
};

#endif // STATEINITIALIZATION_H
