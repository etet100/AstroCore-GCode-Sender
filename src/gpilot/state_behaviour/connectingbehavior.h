// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef CONNECTINGBEHAVIOR_H
#define CONNECTINGBEHAVIOR_H

#include "statebehavior.h"

class ConnectingBehavior : public StateBehavior
{
    public:
        explicit ConnectingBehavior(QObject *parent = nullptr);
        QString name() override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit(StateBehavior *next = nullptr) override;
        void onConnectionStateChanged(ConnectionState state) override;
        void onCommandResponse(QString command, QStringList response) override;

    private:
        bool dataIsReset(QString data);
};

#endif // CONNECTINGBEHAVIOR_H
