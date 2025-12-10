// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef INITIALIZATIONBEHAVIOR_H
#define INITIALIZATIONBEHAVIOR_H

#include "statebehavior.h"

class InitializationBehavior : public StateBehavior
{
    public:
        explicit InitializationBehavior(QObject *parent = nullptr);
        QString description() override { return "Idle"; }
        void onMachineStateChanged(MachineState state) override;
        // bool onCommandResponse(QString command, QString response, QStringList fullResponse) override;
        void onConnectionStateChanged(ConnectionState state) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    protected:
        QString name() const override { return "InitializationBehavior"; }
};

#endif // INITIALIZATIONBEHAVIOR_H
