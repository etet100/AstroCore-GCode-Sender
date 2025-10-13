// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "connectingbehavior.h"
#include <QTimer>
#include "core/communicator/communicator.h"
#include "connectedbehavior.h"

ConnectingBehavior::ConnectingBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void ConnectingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [this, communicator, timer]() {
        if (communicator->connection()->isConnected()) {
            timer->stop();
            timer->deleteLater();
            // emit transition(this, new ConnectedBehavior());

            return;
        }

        communicator->openConnection();
    });
}
