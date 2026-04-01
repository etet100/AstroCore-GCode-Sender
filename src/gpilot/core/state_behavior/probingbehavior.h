// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PROBINGBEHAVIOR_H
#define PROBINGBEHAVIOR_H

#include "statebehavior.h"
#include "core/communicator/proberesponseparser.h"
#include <QVector3D>
#include <optional>
#include <chrono>

class ProbingBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        struct ProbeParameters {
            double fastFeedRate = 50.0;     // Fast probe speed (mm/min)
            double slowFeedRate = 20.0;     // Slow probe speed (mm/min)
            double maxDistance = 5.0;      // Maximum probe distance (mm)
            double retractDistance = 2.0;   // Retract distance between probes (mm)
            double safeDistance = 5.0;      // Safe distance to move up after probing (mm)
            bool doubleProbe = false;       // Whether to do a second slow probe for precision
            bool setZeroAtProbe = true;     // Set Z=0 at probed position
            bool useAbsolute = true;        // Return to absolute positioning

            // Timeouts
            std::chrono::milliseconds setupTimeout{5000};
            std::chrono::milliseconds probeTimeout{60000};
            std::chrono::milliseconds moveTimeout{30000};
        };

        explicit ProbingBehavior(QObject* parent = nullptr);
        explicit ProbingBehavior(ProbeParameters params, QObject* parent = nullptr);
        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

        QVector3D probedPosition() const { return m_probedPosition; }
        bool wasSuccessful() const { return m_success; }

    protected:
        QString name() const override { return "Probing"; }

    private:
        ProbeParameters m_params;
        QVector3D m_probedPosition;
        bool m_success = false;
        QString m_stageDescription = "Setup";

        std::optional<QCoro::Task<void>> m_probingTask;

        QCoro::Task<void> runProbingSequence();

    signals:
        void probeCompleted(QVector3D position);
        void probeFailed(QString reason);
};

#endif // PROBINGBEHAVIOR_H
