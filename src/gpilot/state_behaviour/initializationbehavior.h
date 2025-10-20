// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef INITIALIZATIONBEHAVIOR_H
#define INITIALIZATIONBEHAVIOR_H

#include "statebehavior.h"

class InitializationBehavior : public StateBehavior
{
    public:
        explicit InitializationBehavior(QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        bool isJoggingAllowed() override { return true; } // Jogging should be allowed in idle state
        bool isHomingAllowed() override { return true; } // Homing should be allowed in idle state
        void onDeviceStateChanged(DeviceState state) override;
        // bool onCommandResponse(QString command, QString response, QStringList fullResponse) override;
        void onConnectionStateChanged(ConnectionState state) override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit(StateBehavior *next = nullptr) override;
};

#endif // INITIALIZATIONBEHAVIOR_H
