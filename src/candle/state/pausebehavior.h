// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef D4735774_9D03_4D99_81CA_1470151E54AE
#define D4735774_9D03_4D99_81CA_1470151E54AE
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PAUSEBEHAVIOR_H
#define PAUSEBEHAVIOR_H

#include "statebehavior.h"

class PauseBehavior : public StateBehavior
{
    public:
        // Pause type - different pause sources
        enum class PauseSource {
            Program,      // Pause in G-code program execution
            Jogging,      // Pause in jogging
            UserRequest,  // Manual pause by user
            External      // Pause from external source (e.g. hold signal)
        };

        explicit PauseBehavior(StateBehavior *previous, PauseSource source = PauseSource::Program, QObject *parent = nullptr);
        QString name() override;
        bool isJoggingAllowed() override { return true; } // Jogging is allowed during pause
        bool isHomingAllowed() override { return false; } // Homing is not allowed during pause
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit() override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

        // Pause-specific methods
        void resumeOperation(); // Resume paused operation

    private:
        PauseSource m_source; // Źródło pauzy
};

#endif // PAUSEBEHAVIOR_H


#endif /* D4735774_9D03_4D99_81CA_1470151E54AE */
