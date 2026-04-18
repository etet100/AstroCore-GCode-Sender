// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef SCANTABLEERRORBEHAVIOR_H
#define SCANTABLEERRORBEHAVIOR_H

#include "abstractstatebehavior.h"
#include <QPointF>
#include <optional>

// ScanTableErrorBehavior represents a recoverable failure in the middle of a
// table scan. ScanTableBehavior suspends itself and pushes this behavior when
// a GoTo (or Probe) step fails for a non-alarm reason (timeout, rejected
// command, etc.). The user can then:
//   - Resume: retry the last point — emits resumePrevious(), ScanTableBehavior
//             re-enters and calls processCurrentPoint() again.
//   - Abort:  abandon the scan — transitions to Alarm (if the machine is in
//             Alarm state) or Idle, flushing the suspended stack.
class ScanTableErrorBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        enum class FailedStage {
            Move,
            Probe,
        };

        explicit ScanTableErrorBehavior(QString reason,
                                        FailedStage stage = FailedStage::Move,
                                        std::optional<QPointF> failedPoint = std::nullopt,
                                        int alarmCode = 0,
                                        QObject *parent = nullptr);

        QString description() override;
        Type type() const override { return Type::ScanTableError; }

        QSet<Action::Type> availableActions() const override {
            return {
                Action::Resume,
                Action::Abort,
                Action::Reset,
                Action::Disconnect,
            };
        }

        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;

    protected:
        QString name() const override { return "ScanTableError"; }
        bool doAction(const Action &action) override;

    private:
        QString m_reason;
        FailedStage m_stage;
        std::optional<QPointF> m_failedPoint;
        int m_alarmCode = 0;
        bool m_resumeInProgress = false;

        std::optional<QCoro::Task<void>> m_resumeTask;

        void cancel();
        QCoro::Task<void> runResume();
};

#endif // SCANTABLEERRORBEHAVIOR_H
