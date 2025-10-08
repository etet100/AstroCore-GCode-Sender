// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEHOMING_H
#define STATEHOMING_H

#include "state.h"

class StateHoming : public State
{
    public:
        explicit StateHoming(State *previous, QObject *parent = nullptr);
        QString name() override { return "Homing"; }
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;
};

#endif // STATEHOMING_H
