// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "stateprobing.h"
#include "../communicator.h"

StateProbing::StateProbing(State *previous, QObject *parent): State{previous, parent}
{

}

void StateProbing::onEntry(Communicator *communicator, State *previous)
{
    StateBehavior::onEntry(communicator, previous);

    m_communicator->sendCommands(
        CommandSource::GeneralUI,
        QStringList({
            "G38.2 Z-4 F500",
            // "G91 G21 G38.2 Z-50 F100",
            // "G92 Z14.09",
            // "G0Z5 M30"
        }),
        TABLE_INDEX_UI
    );
}

void StateProbing::onCommandResponse(QString command, QStringList response)
{
    qDebug() << "Probing response: " << response;

    emit transition(this, m_previous);
}
