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

void IdleBehavior::onDeviceStateChanged(DeviceState state)
{
    // Handle device state changes
    if (state == DeviceState::Run) {
        // Machine started running - transition to running behavior
        emit transition(this, new RunningBehavior(this));
    } else if (state == DeviceState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

void IdleBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse)
{
    qDebug() << "[IdleBehavior] Command Response:" << command << response;

    if (command == "$G") {
        m_communicator->processGCodeParserState(commandAttributes, response);
    }
}

void IdleBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[IdleBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);
}
