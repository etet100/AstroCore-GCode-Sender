// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
// #include "pausebehavior.h"
#include "alarmbehavior.h"
// #include "toolchangebehavior.h"

RunningBehavior::RunningBehavior(GCode &program, QObject *parent)
    : StateBehavior{parent}
    , m_feedOverride(100)
    , m_spindleOverride(100)
    , m_program(program)
{}

void RunningBehavior::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Idle) {
        // Program finished or was stopped
        emit transition(this, new IdleBehavior());
    } else if (state == DeviceState::Hold0 || state == DeviceState::Hold1) {
        // Machine is in hold state - transition to pause
        // emit transition(this, new PauseBehavior(PauseBehavior::PauseSource::Program));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

bool RunningBehavior::onCommandResponse(QString command, QString response, QStringList fullResponse)
{

    // Process command responses during running state
    // For example, handle M6 commands for tool change
    if (command.contains("M6")) {
        // Tool change requested
        // emit transition(this, new ToolChangeBehavior(this, command.mid(command.indexOf("T") + 1).toInt(),
        //                                           ToolChangeBehavior::ToolChangeSource::Program));

        return true;
    }

    return false;
}

void RunningBehavior::onAlarm(int code)
{
    // Handle alarm during running state
    emit transition(this, new AlarmBehavior(code));
}

StateBehavior::Result RunningBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[RunningBehavior] Entry";
    return StateBehavior::onEntry(communicator, previous);
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
    // if (m_communicator) {
    //     // Code to send override commands depending on controller type
    //     // m_communicator->sendFeedOverride(percentage);
    // }
}

void RunningBehavior::handleSpindleOverride(int percentage)
{
    // Validate range
    if (percentage < 10) percentage = 10;
    if (percentage > 200) percentage = 200;

    m_spindleOverride = percentage;

    // Send real-time override command to the controller
    if (m_communicator.isNull()) {
        // Code to send spindle override commands
        // m_communicator->sendSpindleOverride(percentage);
    }
}
