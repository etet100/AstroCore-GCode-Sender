// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef DE44D5B5_29D0_4E1A_9C51_8B2BE1B5BB93
#define DE44D5B5_29D0_4E1A_9C51_8B2BE1B5BB93
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef RUNNINGBEHAVIOR_H
#define RUNNINGBEHAVIOR_H

#include "statebehavior.h"

class RunningBehavior : public StateBehavior
{
    public:
        explicit RunningBehavior(QObject *parent = nullptr);
        QString name() override { return "Running"; }
        bool isJoggingAllowed() override { return false; } // Cannot jog while running
        bool isHomingAllowed() override { return false; } // Cannot home while running
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;
        void onAlarm(int code) override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;

        // Running-specific methods
        void handleFeedOverride(int percentage);
        void handleSpindleOverride(int percentage);

    private:
        // Running-specific attributes
        int m_feedOverride;
        int m_spindleOverride;
};

#endif // RUNNINGBEHAVIOR_H


#endif /* DE44D5B5_29D0_4E1A_9C51_8B2BE1B5BB93 */
