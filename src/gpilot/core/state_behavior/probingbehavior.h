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
            // When true, alarms are reported via resumePrevious() + exit data
            // instead of transitioning directly to AlarmBehavior. The caller
            // is then responsible for handling the alarm (e.g. ScanTableBehavior).
            bool delegateAlarmToParent = false;

            // Timeouts
            std::chrono::milliseconds setupTimeout{5000};
            std::chrono::milliseconds probeTimeout{60000};
            std::chrono::milliseconds moveTimeout{30000};
        };

        explicit ProbingBehavior(QObject* parent = nullptr);
        explicit ProbingBehavior(ProbeParameters params, QObject* parent = nullptr);
        QString description() override;
        Type type() const override { return Type::Probing; }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(StateBehavior *next) override;

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

};

#endif // PROBINGBEHAVIOR_H
