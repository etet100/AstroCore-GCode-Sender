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
    StateBehavior::onEntry(communicator, previous);

    if (communicator->connection()->isConnected()) {
        connect(communicator->connection(), &Connection::stateChanged, this, &DisconnectionBehavior::onConnectionStateChanged);
        m_disconnectionTimeoutId = setTimeout(2500, []() {
            qWarning() << "[Behavior][Disconnection] Timeout: connection did not close within 2.5 seconds.";
        });
        communicator->connection()->close();
    } else {
        qDebug() << "[Behavior][Disconnection] Connection is already disconnected";
    }

    return Result::Ok;
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

void DisconnectionBehavior::onConnectionStateChanged(ConnectionState state)
{
    qDebug() << "[Behavior][Disconnection] Connection state changed:" << static_cast<int>(state);
    if (state == ConnectionState::Disconnected) {
        disconnect(m_communicator->connection(), &Connection::stateChanged, this, &DisconnectionBehavior::onConnectionStateChanged);
        clearTimeout(m_disconnectionTimeoutId);
        qDebug() << "[Behavior][Disconnection] Connection is disconnected";
    }
}
