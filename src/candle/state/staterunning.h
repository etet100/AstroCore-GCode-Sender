// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATERUNNING_H
#define STATERUNNING_H

#include "state.h"

class StateRunning : public State
{
    public:
        explicit StateRunning(State *previous, QObject *parent = nullptr);
        QString name() override { return "Running"; }
        bool isJoggingAllowed() override { return false; } // Cannot jog while running
        bool isHomingAllowed() override { return false; } // Cannot home while running
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;
        void onAlarm(int code) override;
};

#endif // STATERUNNING_H
