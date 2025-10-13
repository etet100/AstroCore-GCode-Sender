// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CHECKMODEBEHAVIOR_H
#define CHECKMODEBEHAVIOR_H

#include "statebehavior.h"

class Communicator;

class CheckModeBehavior : public StateBehavior
{
    public:
        explicit CheckModeBehavior(QObject *parent = nullptr);
        QString name() override { return "Check Mode"; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return false; }
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit(StateBehavior *next = nullptr) override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

    private:
        bool m_checkModeEnabled;
};

#endif // CHECKMODEBEHAVIOR_H
