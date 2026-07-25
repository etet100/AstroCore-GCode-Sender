// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "reconnectingbehavior.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

ReconnectingBehavior::ReconnectingBehavior(AbstractConnection *newConnection)
    : AbstractStateBehavior{nullptr}
    , m_newConnection(newConnection)
{}

AbstractStateBehavior::Result ReconnectingBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Reconnecting] Entry";

    if (communicator->connection() && communicator->connection()->isConnected()) {
        qDebug() << "[Behavior][Reconnecting] Closing existing connection...";
        log("Closing existing connection...", QStringList() << "Reconnect" << communicator->connection()->name());

        communicator->connection()->close();

        m_timer = new QTimer(this);
        m_timer->setInterval(50);
        connect(m_timer, &QTimer::timeout, this, [this, communicator]() {
            if (!communicator->connection()->isConnected()) {
                stopTimer();

                qDebug() << "[Behavior][Reconnecting] AbstractConnection closed.";
                log("AbstractConnection closed.", {"Reconnect", communicator->connection()->name()});

                communicator->connection()->deleteLater();
                communicator->setConnection(nullptr, true);

                qDebug() << "[Behavior][Reconnecting] Set new connection and go to ConnectingBehavior.";
                communicator->setConnection(m_newConnection, true);
                emit transition(this, new ConnectingBehavior());

                return;
            }
        });
        m_timer->start();

        // Ok, not WaitForAsyncResult: nothing emits asyncCompleted during entry, so
        // waiting here would leave this behavior out of the state machine forever.
        return Result::Ok;
    }

    qDebug() << "[Behavior][Reconnecting] Set new connection and go straight to ConnectingBehavior.";
    communicator->setConnection(m_newConnection, true);
    emit transition(this, new ConnectingBehavior());

    return Result::Ok;
}
