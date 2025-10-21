// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statehoming.h"
#include "../communicator.h"

StateHoming::StateHoming(State *previous, QObject *parent) : State{previous, parent}
{
}

void StateHoming::onEntry(Communicator *communicator, State *previous)
{
    State::onEntry(communicator, previous);

    m_communicator->sendCommand(CommandSource::GeneralUI, "$H", TABLE_INDEX_UI);
}

void StateHoming::onDeviceStateChanged(DeviceState state)
{
    // if (state == DeviceState::Idle) {
    //     emit transition(this, m_previous);
    // }

    // if (state == DeviceState::Alarm) {
    //     emit error(this, "Homing failed");
    // }
}

void StateHoming::onCommandResponse(QString command, QStringList response)
{
    if (command == "$H") {
        if (response.contains("error")) {
            emit error(this, "Homing failed");
        } else {
            emit transition(this, m_previous);
        }
    }
}
