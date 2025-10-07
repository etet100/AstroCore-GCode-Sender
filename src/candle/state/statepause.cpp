// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statepause.h"
#include "../communicator.h"
#include "staterunning.h"

StatePause::StatePause(State *previous, PauseSource source, QObject *parent)
    : State{previous, parent}
    , m_source(source)
{
}

QString StatePause::name()
{
    QString baseName = "Paused";

    // Add information about pause type
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

void StatePause::onEntry(Communicator *communicator, State *previous)
{
    State::onEntry(communicator, previous);

    // Here we can execute any commands needed when entering pause state
    // For example, we might want to stop the spindle
    // m_communicator->sendCommand(CommandSource::System, "M5", TABLE_INDEX_UI);
}

void StatePause::onExit()
{
    // Clean up any resources or prepare for the next state
}

void StatePause::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Run) {
        // Based on pause source, return to appropriate state
        switch (m_source) {
            case PauseSource::Program:
                // For program pause, return to Running state
                emit transition(this, new StateRunning(this));
                break;
            case PauseSource::Jogging:
                // For jogging pause, return to StateJogging or previous state
                if (dynamic_cast<StateJogging*>(m_previous)) {
                    emit transition(this, m_previous);
                } else {
                    // If previous state is no longer StateJogging, create a new one
                    emit transition(this, new StateJogging(this));
                }
                break;
            case PauseSource::UserRequest:
            case PauseSource::External:
            default:
                // For other sources, return to previous state or Running
                if (m_previous) {
                    emit transition(this, m_previous);
                } else {
                    emit transition(this, new StateRunning(this));
                }
                break;
        }
    }
}

void StatePause::onCommandResponse(QString command, QStringList response)
{
    // Process any command responses during pause state
}