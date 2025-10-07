// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef DA462401_CA87_40BF_8EEE_330C8EBABC2E
#define DA462401_CA87_40BF_8EEE_330C8EBABC2E
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATEPAUSE_H
#define STATEPAUSE_H

#include "state.h"

class StatePause : public State
{
    public:
        // Pause type - different pause sources
        enum class PauseSource {
            Program,      // Pause in G-code program execution
            Jogging,      // Pause in jogging
            UserRequest,  // Manual pause by user
            External      // Pause from external source (e.g. hold signal)
        };

        explicit StatePause(State *previous, PauseSource source = PauseSource::Program, QObject *parent = nullptr);
        QString name() override; // Implementation in .cpp for better readability
        bool isJoggingAllowed() override { return true; } // Allow jogging during pause
        bool isHomingAllowed() override { return false; }
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
        void onExit() override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

    private:
        PauseSource m_source; // Source of the pause
};

#endif // STATEPAUSE_H


#endif /* DA462401_CA87_40BF_8EEE_330C8EBABC2E */
