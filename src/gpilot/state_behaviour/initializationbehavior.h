// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef INITIALIZATIONBEHAVIOR_H
#define INITIALIZATIONBEHAVIOR_H

#include "statebehavior.h"

class InitializationBehavior : public StateBehavior
{
    public:
        explicit InitializationBehavior(QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        void onDeviceStateChanged(DeviceState state) override;
        // bool onCommandResponse(QString command, QString response, QStringList fullResponse) override;
        void onConnectionStateChanged(ConnectionState state) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
};

#endif // INITIALIZATIONBEHAVIOR_H
