// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "disconnectionbehavior.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

DisconnectionBehavior::DisconnectionBehavior(QObject *parent)
    : StateBehavior{parent}
{}

StateBehavior::Result DisconnectionBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[Behavior][Disconnection] Entry";

    return StateBehavior::onEntry(communicator, previous);
}

StateBehavior::Result DisconnectionBehavior::onExit(StateBehavior *next)
{
    qDebug() << "[Behavior][Disconnection] Exit";

    return StateBehavior::onExit(next);
}

bool DisconnectionBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Connect:
            emit transition(this, new InitializationBehavior());

            return true;
    }

    return false;
}
