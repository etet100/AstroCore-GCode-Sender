// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef JOGGINGBEHAVIOR_H
#define JOGGINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/globals.h"

class JoggingBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit JoggingBehavior(JoggindDir direction, double distance, int feedRate, int feedRateZ, QObject *parent = nullptr);
        explicit JoggingBehavior(QVector3D vector, int feedRate, int feedRateZ, QObject *parent = nullptr);
        QString name() override { return "Jogging"; }

        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        bool onAboutToChange(StateBehavior *newState, bool forced) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onMachineStateChanged(MachineState state) override;
        void onMachineState(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

        // Jogging-specific methods
        void startJogging();
        void stopJogging();
        void setJoggingFeedRate(double feedRate);

    private:
        JoggindDir m_currentDirection;
        int m_feedRate;
        int m_feedRateZ;
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
