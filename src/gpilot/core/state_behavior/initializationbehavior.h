// This file is a part of "G-Pilot GCode Sender" application.

#ifndef INITIALIZATIONBEHAVIOR_H
#define INITIALIZATIONBEHAVIOR_H

#include "statebehavior.h"

class InitializationBehavior : public StateBehavior
{
    public:
        explicit InitializationBehavior(QObject *parent = nullptr);
        QString description() override { return "Idle"; }
        Type type() const override { return Type::Initialization; }
        void onMachineStateChanged(MachineState state) override;
        // bool onCommandResponse(QString command, QString response, QStringList fullResponse) override;
        void onConnectionStateChanged(ConnectionState state) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    protected:
        QString name() const override { return "Initialization"; }
};

#endif // INITIALIZATIONBEHAVIOR_H
