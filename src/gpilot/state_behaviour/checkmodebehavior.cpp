// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "../globals.h"
#include "communicator.h"
#include "checkmodebehavior.h"
#include "idlebehavior.h"

CheckModeBehavior::CheckModeBehavior(StateBehavior *previous, QObject *parent)
    : StateBehavior{previous, parent}
    , m_checkModeEnabled(false)
{
}

void CheckModeBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    // Enable check mode using GRBL $C command
    m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
}

void CheckModeBehavior::onExit()
{
    // Disable check mode if it was enabled
    if (m_checkModeEnabled) {
        m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
    }
}

void CheckModeBehavior::onDeviceStateChanged(DeviceState state)
{
    // Handle device state change during check mode
    if (state != DeviceState::Check && m_checkModeEnabled) {
        // Check mode was disabled externally
        m_checkModeEnabled = false;
    }
}

void CheckModeBehavior::onCommandResponse(QString command, QStringList response)
{
    // Process $C command responses
    if (command == "$C") {
        if (!response.contains("error")) {
            // Toggle check mode state
            m_checkModeEnabled = !m_checkModeEnabled;

            // If check mode was just disabled, return to idle
            if (!m_checkModeEnabled) {
                emit transition(this, new IdleBehavior(this));
            }
        }
    }
}
