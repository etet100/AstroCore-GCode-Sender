// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statebehavior.h"
#include "core/communicator/communicator.h"

StateBehavior::StateBehavior(QObject *parent) : QObject(nullptr)
{
}

void StateBehavior::reset()
{
    emit transition(this, new ResetBehavior(this));
}

StateBehavior::Result StateBehavior::onExit(StateBehavior *next)
{
    Q_UNUSED(next);
    stopTimer();
    emit asyncCompleted();

    return Result::Ok;
}

void StateBehavior::stopTimer()
{
    if (m_timer) {
        m_timer->stop();
        m_timer->deleteLater();
        m_timer = nullptr;
    }
}

StateBehavior::Result StateBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    if (previous) {
        m_previous = previous;
    }

    m_communicator = communicator;

    return Result::Ok;
}

void StateBehavior::waitForStateResponse(StateResponseCallback callback)
{
    m_stateResponseCallbacks.append(callback);
}

void StateBehavior::log(QString message)
{
    emit logSignal(message);
}
