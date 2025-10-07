// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef B745AA0B_573F_46AC_9074_CE149A678FEB
#define B745AA0B_573F_46AC_9074_CE149A678FEB
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef IDLEBEHAVIOR_H
#define IDLEBEHAVIOR_H

#include "statebehavior.h"

class IdleBehavior : public StateBehavior
{
    public:
        explicit IdleBehavior(StateBehavior *previous, QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        bool isJoggingAllowed() override { return true; } // Jogging should be allowed in idle state
        bool isHomingAllowed() override { return true; } // Homing should be allowed in idle state
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;
};

#endif // IDLEBEHAVIOR_H


#endif /* B745AA0B_573F_46AC_9074_CE149A678FEB */
