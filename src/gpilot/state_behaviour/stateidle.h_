// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEIDLE_H
#define STATEIDLE_H

#include "state.h"

class StateIdle : public State
{
    public:
        explicit StateIdle(State *previous, QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        bool isJoggingAllowed() override { return true; } // Jogging should be allowed in idle state
        bool isHomingAllowed() override { return true; } // Homing should be allowed in idle state
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;
};

#endif // STATEIDLE_H
