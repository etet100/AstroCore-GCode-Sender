// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#ifndef JOGGINGBEHAVIOR_H
#define JOGGINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/globals.h"
#include <QElapsedTimer>

class JoggingBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit JoggingBehavior(QVector3D vector, double distance, bool continuous, int feedRate, int feedRateZ, QObject *parent = nullptr);
        explicit JoggingBehavior(int feedRate, int feedRateZ, QObject *parent = nullptr);
        QString description() override { return "Jogging"; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Stop, Action::Jog };
        }
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        bool onAboutToChange(StateBehavior *newState, bool forced) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onMachineStateChanged(MachineState state) override;
        void onMachineState(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

        // Jogging-specific methods
        void startJogging();
        void stopJogging();
        void setJoggingFeedRate(double feedRate);

    protected:
        QString name() const override { return "Jogging"; }
        bool doAction(const Action &action) override;

    private:
        static constexpr int TIMER_INTERVAL_MS = 50;

        QVector3D m_joggingVector;
        int m_feedRate;
        int m_feedRateZ;
        double m_distance;
        bool m_continuous = false;
        bool m_isJogging = false;
        bool m_isJoggingState = false;
        bool m_firstCommand = true;
        bool m_stopping = false;
        QString m_jogCommand;
        QVector3D m_startMachinePos;
        QTimer m_joggingTimer;
        int m_sent = 0;
        int m_acked = 0;
        double m_segmentDist = 0.0;
        double m_targetLookahead = 0.0;
        QElapsedTimer m_fillBufferLogTimer;

        void continueJogging();
        void fillBuffer();
        void buildJogCommand(double distance);
};

#endif // JOGGINGBEHAVIOR_H
