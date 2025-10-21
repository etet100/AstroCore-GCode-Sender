// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "state.h"

State::State(State *previous, QObject *parent) : QObject{parent}, m_previous{previous}
{
}

void State::onEntry(Communicator *communicator, State *previous) {
    if (previous) {
        m_previous = previous;
    }
    m_communicator = communicator;
}
