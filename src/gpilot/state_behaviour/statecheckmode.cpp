// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statecheckmode.h"
#include "../communicator.h"
#include "stateidle.h"

StateCheckMode::StateCheckMode(State *previous, QObject *parent)
    : State{previous, parent}
    , m_checkModeEnabled(false)
{
}

void StateCheckMode::onEntry(Communicator *communicator, State *previous)
{
    State::onEntry(communicator, previous);

    // Enable check mode (GRBL $C command)
    m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
}

void StateCheckMode::onExit()
{
    // Disable check mode if it was enabled
    if (m_checkModeEnabled) {
        m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
    }
}

void StateCheckMode::onDeviceStateChanged(DeviceState state)
{
    // Handle changes in device state during check mode
    if (state != DeviceState::Check && m_checkModeEnabled) {
        // Check mode was disabled externally
        m_checkModeEnabled = false;
    }
}

void StateCheckMode::onCommandResponse(QString command, QStringList response)
{
    // Process responses to $C commands
    if (command == "$C") {
        if (!response.contains("error")) {
            // Toggle check mode state
            m_checkModeEnabled = !m_checkModeEnabled;

            // If check mode was just disabled, return to idle state
            if (!m_checkModeEnabled) {
                emit transition(this, new StateIdle(this));
            }
        }
    }
}