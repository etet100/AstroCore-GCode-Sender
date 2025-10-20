// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef HOMINGBEHAVIOR_H
#define HOMINGBEHAVIOR_H

#include "statebehavior.h"

class HomingBehavior : public StateBehavior
{
    public:
        explicit HomingBehavior(QObject *parent = nullptr);
        QString name() override { return "Homing"; }
        bool isJoggingAllowed() override { return false; } // Cannot jog during homing
        bool isHomingAllowed() override { return false; } // Cannot start homing when already in progress
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onDeviceStateChanged(DeviceState state) override;
        bool onCommandResponse(QString command, QString response, QStringList fullResponse) override;

    private:
        bool m_homingStarted;
        bool m_homingCompleted;
};

#endif // HOMINGBEHAVIOR_H

