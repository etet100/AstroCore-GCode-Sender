// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "staterunning.h"
#include "stateidle.h"
#include "statepause.h"
#include "statealarm.h"
#include "../communicator.h"

StateRunning::StateRunning(State *previous, QObject *parent)
    : State{previous, parent}
{}

void StateRunning::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Idle) {
        // Program finished or was stopped
        emit transition(this, new StateIdle(this));
    } else if (state == DeviceState::Hold0 || state == DeviceState::Hold1) {
        // Machine is in hold state - transition to pause
        // Używamy typu pauzy Program, ponieważ jesteśmy w stanie Running
        emit transition(this, new StatePause(this, StatePause::PauseSource::Program));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new StateAlarm(this));
    }
}

void StateRunning::onCommandResponse(QString command, QStringList response)
{
    // Process command responses during running state
    // For example, handle M6 commands for tool change
    if (command.contains("M6")) {
        // Tool change requested - używamy typu Program, ponieważ zmiana narzędzia
        // pochodzi z programu G-code
        emit transition(this, new StateToolChange(this, command.mid(command.indexOf("T") + 1).toInt(),
                                                StateToolChange::ToolChangeSource::Program));
    }
}

void StateRunning::onAlarm(int code)
{
    // Handle alarm during running state
    emit transition(this, new StateAlarm(this, code));
}
