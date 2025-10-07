// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statetoolchange.h"
#include "../communicator.h"
#include "staterunning.h"

StateToolChange::StateToolChange(State *previous, int toolNumber,
                               ToolChangeSource source, QObject *parent)
    : State{previous, parent}
    , m_toolNumber(toolNumber)
    , m_toolChangeConfirmed(false)
    , m_source(source)
{
}

void StateToolChange::onEntry(Communicator *communicator, State *previous)
{
    State::onEntry(communicator, previous);

    // In a manual tool change workflow, we might want to pause and wait for user input
    // This could involve showing a dialog or other UI element

    // Emit a signal that could be connected to the UI to show a tool change dialog
    // emit toolChangeRequested(m_toolNumber);
}

void StateToolChange::onExit()
{
    // Clean up resources or finalize the tool change process
}

void StateToolChange::onDeviceStateChanged(DeviceState state)
{
    // Handle changes in device state during tool change
    if (state == DeviceState::Run && m_toolChangeConfirmed) {
        // If tool change was from program, go back to Running state
        // Otherwise return to the previous state (usually Idle)
        if (m_source == ToolChangeSource::Program) {
            emit transition(this, new StateRunning(this));
        } else {
            // Return to previous state or create new Idle state
            // if previous state is not appropriate
            emit transition(this, m_previous ? m_previous : new StateIdle(this));
        }
    }
}

void StateToolChange::onCommandResponse(QString command, QStringList response)
{
    // Process command responses during tool change
    // For example, handle M6 command completion
    if (command.contains("M6") && !response.contains("error")) {
        m_toolChangeConfirmed = true;
    }
}