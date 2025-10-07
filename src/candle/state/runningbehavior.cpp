// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "runningbehavior.h"
#include "idlebehavior.h"
#include "pausebehavior.h"
#include "alarmbehavior.h"
#include "toolchangebehavior.h"
#include "../communicator.h"

RunningBehavior::RunningBehavior(StateBehavior *previous, QObject *parent)
    : StateBehavior{previous, parent}
    , m_feedOverride(100)
    , m_spindleOverride(100)
{}

void RunningBehavior::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Idle) {
        // Program finished or was stopped
        emit transition(this, new IdleBehavior(this));
    } else if (state == DeviceState::Hold0 || state == DeviceState::Hold1) {
        // Machine is in hold state - transition to pause
        emit transition(this, new PauseBehavior(this, PauseBehavior::PauseSource::Program));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior(this));
    }
}

void RunningBehavior::onCommandResponse(QString command, QStringList response)
{
    // Process command responses during running state
    // For example, handle M6 commands for tool change
    if (command.contains("M6")) {
        // Tool change requested
        emit transition(this, new ToolChangeBehavior(this, command.mid(command.indexOf("T") + 1).toInt(),
                                                  ToolChangeBehavior::ToolChangeSource::Program));
    }
}

void RunningBehavior::onAlarm(int code)
{
    // Handle alarm during running state
    emit transition(this, new AlarmBehavior(this, code));
}

void RunningBehavior::handleFeedOverride(int percentage)
{
    // Validate range
    if (percentage < 10) percentage = 10;
    if (percentage > 200) percentage = 200;

    m_feedOverride = percentage;

    // Send real-time override command to the controller
    // Assuming we have methods to send real-time override commands
    // for GRBL controllers
    if (m_communicator) {
        // Code to send override commands depending on controller type
        // m_communicator->sendFeedOverride(percentage);
    }
}

void RunningBehavior::handleSpindleOverride(int percentage)
{
    // Validate range
    if (percentage < 10) percentage = 10;
    if (percentage > 200) percentage = 200;

    m_spindleOverride = percentage;

    // Send real-time override command to the controller
    if (m_communicator) {
        // Code to send spindle override commands
        // m_communicator->sendSpindleOverride(percentage);
    }
}