// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef JOGGINGBEHAVIOR_H
#define JOGGINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/globals.h"

class JoggingBehavior : public StateBehavior
{
    public:
        explicit JoggingBehavior(JoggindDir direction, double distance, int feedRate, QObject *parent = nullptr);
        explicit JoggingBehavior(QVector3D vector, int feedRate,  QObject *parent = nullptr);
        QString name() override { return "Jogging"; }
        bool isJoggingAllowed() override { return true; } // Jogging is allowed in this state
        bool isHomingAllowed() override { return false; } // Cannot home during jogging
        bool isNewStateAllowed(StateBehavior *newState) override;

        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onExit(StateBehavior *next = nullptr) override;
        void onDeviceStateChanged(DeviceState state) override;
        void onDeviceState(DeviceState state) override;
        void onCommandResponse(QString command, QString response, QStringList fullResponse) override;

        // Jogging-specific methods
        void startJogging();
        void stopJogging();
        void setJoggingFeedRate(double feedRate);

    private:
        JoggindDir m_currentDirection;
        int m_feedRate;
        double m_distance; // 0 means continuous jogging
        bool m_isJogging = false;
        bool m_isJoggingState = false;
        bool m_firstCommand = true;
        bool m_stopping = false;
        QString m_jogCommand;
        QVector3D m_vector;
        QTimer m_joggingTimer;
        int m_sent = 0;
        int m_acked = 0;
        void continueJogging();
};

#endif // JOGGINGBEHAVIOR_H
