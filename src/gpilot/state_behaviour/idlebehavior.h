// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef IDLEBEHAVIOR_H
#define IDLEBEHAVIOR_H

#include "statebehavior.h"

class IdleBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit IdleBehavior(QObject *parent = nullptr);
        QString description() override { return "Idle"; }
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    protected:
        QString name() const override { return "IdleBehavior"; }
        bool doAction(const Action &action) override;

};

#endif // IDLEBEHAVIOR_H
