// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2024 BTS

#include "reconnectingbehavior.h"
#include "core/communicator/communicator.h"

ReconnectingBehavior::ReconnectingBehavior(Connection *newConnection)
    : StateBehavior{nullptr}
    , m_newConnection(newConnection)
{}

void ReconnectingBehavior::onConnectionStateChanged(ConnectionState state)
{
    if (m_stage == Closing) {
        if (state == ConnectionState::Disconnected) {
            qDebug() << "[ReconnectingBehavior] Connection closed.";

            emit transition(this, new ConnectingBehavior());

            m_stage = Completed;
        }
    }
}

void ReconnectingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[ReconnectingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    if (!communicator->connection()->isConnected()) {
        m_stage = Completed;
        qDebug() << "[ReconnectingBehavior] No active connection. Proceeding to connect.";
        emit transition(this, new ConnectingBehavior());
    } else {
        m_stage = Closing;
        qDebug() << "[ReconnectingBehavior] Closing current connection.";
        communicator->connection()->close();
    }
}
