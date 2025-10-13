// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef D1DB2AB9_9615_41B9_8224_AC77F9B9B431
#define D1DB2AB9_9615_41B9_8224_AC77F9B9B431
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef JOGGINGBEHAVIOR_H
#define JOGGINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/globals.h"

class JoggingBehavior : public StateBehavior
{
    public:
        explicit JoggingBehavior(QObject *parent = nullptr);
        QString name() override { return "Jogging"; }
        bool isJoggingAllowed() override { return true; } // Jogging is allowed in this state
        bool isHomingAllowed() override { return false; } // Cannot home during jogging

        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit(StateBehavior *next = nullptr) override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QStringList response) override;

        // Jogging-specific methods
        void startJogging(JoggindDir direction, double feedRate, double distance = 0);
        void stopJogging();
        void setJoggingFeedRate(double feedRate);

    private:
        JoggindDir m_currentDirection;
        double m_feedRate;
        double m_distance; // 0 means continuous jogging
        bool m_isJogging;
};

#endif // JOGGINGBEHAVIOR_H


#endif /* D1DB2AB9_9615_41B9_8224_AC77F9B9B431 */
