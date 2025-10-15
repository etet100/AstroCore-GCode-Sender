// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef IDLEBEHAVIOR_H
#define IDLEBEHAVIOR_H

#include "statebehavior.h"

class IdleBehavior : public StateBehavior
{
    public:
        explicit IdleBehavior(QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        bool isJoggingAllowed() override { return true; } // Jogging should be allowed in idle state
        bool isHomingAllowed() override { return true; } // Homing should be allowed in idle state
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, CommandAttributes commandAttributes, QStringList response) override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
};

#endif // IDLEBEHAVIOR_H
