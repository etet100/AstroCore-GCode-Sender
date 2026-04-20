// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "disconnectingbehavior.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

DisconnectingBehavior::DisconnectingBehavior(QObject *parent)
    : AbstractStateBehavior{parent}
{}

AbstractStateBehavior::Result DisconnectingBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Disconnecting] Entry";

    if (communicator->connection()->isConnected()) {
        connect(communicator->connection(), &AbstractConnection::stateChanged, this, &DisconnectingBehavior::onConnectionStateChanged);
        m_disconnectionTimeoutId = setTimeout(2500, []() {
            qWarning() << "[Behavior][Disconnecting] Timeout: connection did not close within 2.5 seconds.";
        });
        communicator->connection()->close();
    } else {
        qDebug() << "[Behavior][Disconnecting] AbstractConnection is already disconnected";
    }

    return Result::Ok;
}

AbstractStateBehavior::Result DisconnectingBehavior::doOnExit(AbstractStateBehavior *next)
{
    qDebug() << "[Behavior][Disconnecting] Exit";

    return Result::Ok;
}

bool DisconnectingBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Connect:
            emit transition(this, new InitializationBehavior());

            return true;
    }

    return false;
}

void DisconnectingBehavior::onConnectionStateChanged(ConnectionState state)
{
    qDebug() << "[Behavior][Disconnecting] AbstractConnection state changed:" << static_cast<int>(state);
    if (state == ConnectionState::Disconnected) {
        disconnect(m_communicator->connection(), &AbstractConnection::stateChanged, this, &DisconnectingBehavior::onConnectionStateChanged);
        clearTimeout(m_disconnectionTimeoutId);
        qDebug() << "[Behavior][Disconnecting] AbstractConnection is disconnected";
    }
}
