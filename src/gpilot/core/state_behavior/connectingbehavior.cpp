// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "connectingbehavior.h"
#include <QTimer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

ConnectingBehavior::ConnectingBehavior(QObject *parent)
    : StateBehavior{parent}
{}

QString ConnectingBehavior::description() { return "Connecting"; }

StateBehavior::Result ConnectingBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[ConnectingBehavior] Entry, attempting to connect...";
    StateBehavior::onEntry(communicator, previous);

    assert(communicator->connection() != nullptr);

    if (communicator->connection()->open() && communicator->connection()->isConnected()) {
        log(QString("[Connecting][%1] Connected").arg(communicator->connection()->name()));

        return Result::Ok;
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(2000);
    connect(m_timer, &QTimer::timeout, this, [this, communicator]() {
        qDebug() << "[ConnectingBehavior] Attempting to connect...";
        log(QString("[Connecting][%1] Attempting to connect...").arg(communicator->connection()->name()));
        if (communicator->connection()->isConnected()) {
            stopTimer();

            return;
        }

        communicator->connection()->open();
    });
    m_timer->start();

    return Result::Ok;
}

StateBehavior::Result ConnectingBehavior::onExit(StateBehavior *next)
{
    qDebug() << "[ConnectingBehavior] Exiting.";

    stopTimer();
    if (!m_communicator->connection()->isConnected()) {
        qDebug() << "[ConnectingBehavior] Connection not established. Giving up.";
        log("Connection not established. Giving up.", {"Connecting", m_communicator->connection()->name()});

        // m_communicator->connection()->deleteLater();
        // m_communicator->setConnection(nullptr, true);
    }

    return StateBehavior::onExit(next);
}

void ConnectingBehavior::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Connected) {
        stopTimer();
        qDebug() << "[ConnectingBehavior] Connected.";

        emit transition(this, new ResetBehavior());
    }
}
