// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEPROBING_H
#define STATEPROBING_H

#include "state.h"

class StateProbing : public State
{
    public:
        StateProbing(State *previous, QObject *parent = nullptr);
        QString name() override { return "Probing"; }
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
        void onCommandResponse(QString command, QStringList response) override;
};

#endif // STATEPROBING_H
