// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEJOGGINGWAITINGFORIDLE_H
#define STATEJOGGINGWAITINGFORIDLE_H

#include "state.h"

class StateJoggingWaitingForIdle : public State
{
    public:
        explicit StateJoggingWaitingForIdle(State *previous, QObject *parent = nullptr);
};

#endif // STATEJOGGINGWAITINGFORIDLE_H
