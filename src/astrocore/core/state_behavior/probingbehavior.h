// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PROBINGBEHAVIOR_H
#define PROBINGBEHAVIOR_H

#include "abstractstatebehavior.h"
#include "core/communicator/proberesponseparser.h"
#include <QVector3D>
#include <optional>
#include <chrono>

class ProbingBehavior : public AbstractStateBehavior
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
            // When true, the sequence starts with a `G0 Z<safeDistance>` retract
            // before the first probe. Used when resuming after a probe-time
            // alarm so the tip is lifted off the workpiece before probing again.
            bool retractFirst = false;

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
        Result doOnExit(AbstractStateBehavior *next) override;
        void onAlarm(int code) override;

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

        // Waits for motion to start (up to 500ms for Run) and then for Idle.
        // On alarm or timeout, emits the appropriate transition/resumePrevious
        // and returns false — the caller should co_return.
        QCoro::Task<bool> waitForMotionComplete(const QString &stage);

        // Sends G38.2 probe command, waits for response, validates it and parses
        // the contact position. On any error the method emits the appropriate
        // transition/resumePrevious and returns nullopt — the caller should co_return.
        QCoro::Task<std::optional<QVector3D>> executeProbe(
            const QString &stage, double distance, double feedRate);

        // Routes an active alarm either to the parent (when delegateAlarmToParent)
        // or to a fresh AlarmBehavior. Caller is expected to log first.
        void emitAlarmExit();

        // Non-alarm failure exit: optionally restores absolute mode and resumes.
        QCoro::Task<void> emitFailureExit();

};

#endif // PROBINGBEHAVIOR_H
