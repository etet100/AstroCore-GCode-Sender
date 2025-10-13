// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "connectingbehavior.h"
#include <QTimer>
#include "core/communicator/communicator.h"

ConnectingBehavior::ConnectingBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void ConnectingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "Attempting to connect...";
    if (communicator->connection()->open() && communicator->connection()->isConnected()) {
        return;
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, [this, communicator]() {
        qDebug() << "Attempting to connect...";
        if (communicator->connection()->isConnected()) {
            stopTimer();

            return;
        }

        communicator->connection()->open();
    });
    m_timer->start();
}

void ConnectingBehavior::onExit(StateBehavior *next)
{
    StateBehavior::onExit(next);
}

void ConnectingBehavior::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Connected) {
        stopTimer();
        emit transition(this, new IdleBehavior());
    }
}
