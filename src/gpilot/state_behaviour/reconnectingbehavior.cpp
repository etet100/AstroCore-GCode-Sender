// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2024 BTS

#include "reconnectingbehavior.h"
#include "core/communicator/communicator.h"

ReconnectingBehavior::ReconnectingBehavior(Connection *newConnection)
    : StateBehavior{nullptr}
    , m_newConnection(newConnection)
{}

StateBehavior::Result ReconnectingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[ReconnectingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    if (m_communicator->connection() && m_communicator->connection()->isConnected()) {
        qDebug() << "[ReconnectingBehavior] Closing existing connection...";
        log("Closing existing connection...", QStringList() << "Reconnect" << communicator->connection()->name());

        m_communicator->connection()->close();

        m_timer = new QTimer(this);
        m_timer->setInterval(50);
        connect(m_timer, &QTimer::timeout, this, [this, communicator]() {
            if (!communicator->connection()->isConnected()) {
                stopTimer();

                qDebug() << "[ReconnectingBehavior] Connection closed.";
                log("Connection closed.", {"Reconnect", communicator->connection()->name()});

                m_communicator->connection()->deleteLater();
                m_communicator->m_connection = nullptr;

                qDebug() << "[ReconnectingBehavior] Set new connection and go to ConnectingBehavior.";
                communicator->setConnection(m_newConnection, true);
                emit transition(this, new ConnectingBehavior());

                return;
            }
        });
        m_timer->start();

        return Result::WaitForAsyncResult;
    }

    qDebug() << "[ReconnectingBehavior] Set new connection and go straight to ConnectingBehavior.";
    communicator->setConnection(m_newConnection, true);
    emit transition(this, new ConnectingBehavior());

    return Result::Ok;
}
