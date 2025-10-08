// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef B564CB86_313F_4101_AA7A_D397501ED8FB
#define B564CB86_313F_4101_AA7A_D397501ED8FB
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef STATECHECKMODE_H
#define STATECHECKMODE_H

#include "state.h"

class StateCheckMode : public State
{
    public:
        explicit StateCheckMode(State *previous, QObject *parent = nullptr);
        QString name() override { return "Check Mode"; }
        bool isJoggingAllowed() override { return false; }
        bool isHomingAllowed() override { return false; }
        void onEntry(Communicator *communicator, State *previous = nullptr) override;
        void onExit() override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

    private:
        bool m_checkModeEnabled;
};

#endif // STATECHECKMODE_H


#endif /* B564CB86_313F_4101_AA7A_D397501ED8FB */
