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

    // Continuous jogging buffer strategy:
    //
    // segmentDist  = clamp(speed_mm_per_ms * timerInterval, minSegment, maxSegment)
    // lookahead    = clamp(speed_mm_per_ms * targetBufferTime, minLookahead, maxLookahead)
    //
    // Fast profile (30ms timer, for responsive senders):
    //   Feed  | Segment | Lookahead | Buffer time | Segments ahead
    //   F300  | 0.5mm   | 3.0mm     | ~600ms      | ~6
    //   F1000 | 0.5mm   | 5.8mm     | ~350ms      | ~12
    //   F2500 | 1.25mm  | 14.6mm    | ~350ms      | ~12
    //   F5000 | 2.5mm   | 25mm      | ~300ms      | ~10
    //   F8000 | 3.0mm   | 25mm      | ~188ms      | ~8
    //
    // Relaxed profile (100ms timer, for slower senders):
    //   Feed  | Segment | Lookahead | Buffer time | Segments ahead
    //   F300  | 1.0mm   | 5.0mm     | ~1000ms     | ~5
    //   F1000 | 1.67mm  | 8.3mm     | ~500ms      | ~5
    //   F2500 | 4.17mm  | 20.8mm    | ~500ms      | ~5
    //   F5000 | 5.0mm   | 30mm      | ~360ms      | ~6
    //   F8000 | 5.0mm   | 30mm      | ~225ms      | ~6
    //
    // fillBuffer() is called every timerInterval and sends segments until
    // the distance-based lookahead is reached or the serial buffer (127B) is full.
    // Smaller segments = more fit in serial buffer = GRBL planner sees further ahead.
    // Jog cancel empties the planner, so max stop distance = lookahead.

    public:
        struct BufferProfile {
            int timerIntervalMs;
            double minSegmentMm;
            double maxSegmentMm;
            int targetBufferTimeMs;
            double minLookaheadMm;
            double maxLookaheadMm;
        };

        static constexpr BufferProfile Fast    {  30, 0.5, 3.0, 350, 3.0, 25.0 };
        static constexpr BufferProfile Relaxed { 100, 1.0, 5.0, 500, 5.0, 30.0 };

        explicit JoggingBehavior(QVector3D vector, double distance, bool continuous, int feedRate, int feedRateZ, QObject *parent = nullptr);
        explicit JoggingBehavior(int feedRate, int feedRateZ, QObject *parent = nullptr);
        QString description() override { return "Jogging"; }
        Type type() const override { return Type::Jogging; }
        QSet<Action::Type> availableActions() const override {
            return {
                Action::Reset,
                Action::Abort,
                Action::Jog
            };
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
        void setBufferProfile(const BufferProfile &profile) { m_profile = profile; }

    protected:
        QString name() const override { return "Jogging"; }
        bool doAction(const Action &action) override;

    private:
        BufferProfile m_profile = Relaxed;

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
