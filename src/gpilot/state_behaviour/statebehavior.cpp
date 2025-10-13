// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statebehavior.h"

StateBehavior::StateBehavior(QObject *parent) : QObject{parent}
{
}

void StateBehavior::onEntry(Communicator *communicator, StateBehavior *previous) {
    if (previous) {
        m_previous = previous;
    }
    m_communicator = communicator;
}
