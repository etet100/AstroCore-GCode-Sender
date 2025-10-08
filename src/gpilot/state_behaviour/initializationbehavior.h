// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef INITIALIZATIONBEHAVIOR_H
#define INITIALIZATIONBEHAVIOR_H

#include "statebehavior.h"

class InitializationBehavior : public StateBehavior
{
    public:
        explicit InitializationBehavior(StateBehavior *previous, QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        bool isJoggingAllowed() override { return true; } // Jogging should be allowed in idle state
        bool isHomingAllowed() override { return true; } // Homing should be allowed in idle state
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;
};

#endif // INITIALIZATIONBEHAVIOR_H
