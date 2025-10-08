// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "../globals.h"
#include "communicator.h"
#include "pausebehavior.h"
#include "runningbehavior.h"
#include "joggingbehavior.h"

PauseBehavior::PauseBehavior(StateBehavior *previous, PauseSource source, QObject *parent)
    : StateBehavior{previous, parent}
    , m_source(source)
{
}

QString PauseBehavior::name()
{
    QString baseName = "Paused";

    // Add pause type information
    switch (m_source) {
        case PauseSource::Program:
            return baseName + " (Program)";
        case PauseSource::Jogging:
            return baseName + " (Jogging)";
        case PauseSource::UserRequest:
            return baseName + " (User Requested)";
        case PauseSource::External:
            return baseName + " (External Hold)";
        default:
            return baseName;
    }
}

void PauseBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    // Perform different actions based on pause source
    switch (m_source) {
        case PauseSource::Program:
            // For example, stop spindle during program pause
            // m_communicator->sendCommand(CommandSource::System, "M5", TABLE_INDEX_UI);
            break;
        case PauseSource::Jogging:
            // During jogging pause, may want to take other actions
            break;
        default:
            break;
    }
}

void PauseBehavior::onExit()
{
    // Clean up resources or prepare for next state
}

void PauseBehavior::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Run) {
        // Based on pause source, return to appropriate state
        switch (m_source) {
            case PauseSource::Program:
                // For program pause, return to Running state
                emit transition(this, new RunningBehavior(this));
                break;
            case PauseSource::Jogging:
                // For jogging pause, return to Jogging state
                emit transition(this, new JoggingBehavior(this));
                break;
            case PauseSource::UserRequest:
            case PauseSource::External:
            default:
                // For other sources, return to previous state or Running
                if (m_previous) {
                    emit transition(this, m_previous);
                } else {
                    emit transition(this, new RunningBehavior(this));
                }
                break;
        }
    }
}

void PauseBehavior::onCommandResponse(QString command, QStringList response)
{
    // Process command responses in pause state
}

void PauseBehavior::resumeOperation()
{
    // Send resume (cycle start) command to the controller
    if (m_communicator) {
        m_communicator->sendRealtimeCommand(GRBL_LIVE_CYCLE_START);
    }
}
