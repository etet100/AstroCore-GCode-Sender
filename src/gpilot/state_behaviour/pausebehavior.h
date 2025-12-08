// This file is a part of "G-Pilot (formerly Candle)" application.
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

        explicit PauseBehavior(PauseSource source = PauseSource::Program, QObject *parent = nullptr);
        bool action(const Action &action) override;
        QString name() override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
    private:
        PauseSource m_source;
        void resume();
};

#endif // PAUSEBEHAVIOR_H

