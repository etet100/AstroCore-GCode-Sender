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
    : AbstractStateBehavior{parent}
{}

QString ConnectingBehavior::description() { return "Connecting"; }

AbstractStateBehavior::Result ConnectingBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Connecting] Entry, attempting to connect...";

    assert(communicator->connection() != nullptr);

    if (communicator->connection()->open() && communicator->connection()->isConnected()) {
        log(QString("[Connecting][%1] Connected").arg(communicator->connection()->name()));

        return Result::Ok;
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(2000);
    connect(m_timer, &QTimer::timeout, this, [this, communicator]() {
        qDebug() << "[Behavior][Connecting] Attempting to connect...";
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

AbstractStateBehavior::Result ConnectingBehavior::doOnExit(AbstractStateBehavior *next)
{
    qDebug() << "[Behavior][Connecting] Exiting.";

    stopTimer();
    if (!m_communicator->connection()->isConnected()) {
        qDebug() << "[Behavior][Connecting] AbstractConnection not established. Giving up.";
        log("AbstractConnection not established. Giving up.", {"Connecting", m_communicator->connection()->name()});

        // m_communicator->connection()->deleteLater();
        // m_communicator->setConnection(nullptr, true);
    }

    return Result::Ok;
}

void ConnectingBehavior::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Connected) {
        stopTimer();
        qDebug() << "[Behavior][Connecting] Connected.";

        bool resetFirst = m_configuration->senderModule().resetAfterConnecting();
        if (resetFirst) {
            emit transition(this, new ResetBehavior());
        } else {
            emit transition(this, new HandshakeBehavior());
        }
    }
}
