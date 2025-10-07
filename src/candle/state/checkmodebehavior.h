// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef D2D1EF6D_92B6_4349_886F_9253649C2E8C
#define D2D1EF6D_92B6_4349_886F_9253649C2E8C
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CHECKMODEBEHAVIOR_H
#define CHECKMODEBEHAVIOR_H

#include "statebehavior.h"

class CheckModeBehavior : public StateBehavior
{
    public:
        explicit CheckModeBehavior(StateBehavior *previous, QObject *parent = nullptr);
        QString name() override { return "Check Mode"; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return false; }
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit() override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

    private:
        bool m_checkModeEnabled;
};

#endif // CHECKMODEBEHAVIOR_H


#endif /* D2D1EF6D_92B6_4349_886F_9253649C2E8C */
