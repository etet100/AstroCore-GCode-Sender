// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "runningbehavior.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"
#include "core/communicator/communicator.h"

IdleBehavior::IdleBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void IdleBehavior::onMachineStateChanged(MachineState state)
{
    // Handle device state changes
    if (state == MachineState::Run) {
        // Machine started running - transition to running behavior
        // emit transition(this, new RunningBehavior(this));
    } else if (state == MachineState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

bool IdleBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse)
{
    assert(m_communicator != nullptr && !m_communicator.isNull());

    qDebug() << "[IdleBehavior] Command Response:" << command << response;

    if (command == "$G") {
        m_communicator->processGCodeParserState(commandAttributes, response);

        return true;
    }

    return false;
}

StateBehavior::Result IdleBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[IdleBehavior] Entry";
    return StateBehavior::onEntry(communicator, previous);
}
