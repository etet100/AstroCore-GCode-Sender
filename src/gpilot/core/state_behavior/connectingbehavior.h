// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef CONNECTINGBEHAVIOR_H
#define CONNECTINGBEHAVIOR_H

#include "statebehavior.h"

class ConnectingBehavior : public StateBehavior
{
    public:
        explicit ConnectingBehavior(QObject *parent = nullptr);
        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onConnectionStateChanged(ConnectionState state) override;

    protected:
        QString name() const override { return "Connecting"; }

    private:
        // bool dataIsReset(QString data);
};

#endif // CONNECTINGBEHAVIOR_H
