// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "scantableerrorbehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

using namespace std::chrono_literals;

ScanTableErrorBehavior::ScanTableErrorBehavior(QString reason,
                                               FailedStage stage,
                                               std::optional<QPointF> failedPoint,
                                               int alarmCode,
                                               QObject *parent)
    : AbstractStateBehavior{parent}
    , m_reason(std::move(reason))
    , m_stage(stage)
    , m_failedPoint(failedPoint)
    , m_alarmCode(alarmCode)
{}

QString ScanTableErrorBehavior::description()
{
    QString stageLabel = (m_stage == FailedStage::Probe) ? "pro" : "mov";

    return QString("Scan failed %1/%2").arg(stageLabel, m_reason);
}

AbstractStateBehavior::Result ScanTableErrorBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][ScanTableError] Entry — stage:"
             << (m_stage == FailedStage::Probe ? "probe" : "move")
             << "reason:" << m_reason
             << "alarmCode:" << m_alarmCode;
    log(QString("Scan paused: %1").arg(m_reason), {"ScanTableError"});

    emit stateEvent("scanPaused", {
        {"reason", m_reason},
        {"stage", m_stage == FailedStage::Probe ? "probe" : "move"},
        {"alarmCode", m_alarmCode},
    });

    m_communicator->startQueryingMachineState();

    return Result::Ok;
}

AbstractStateBehavior::Result ScanTableErrorBehavior::doOnExit(AbstractStateBehavior *next)
{
    qDebug() << "[Behavior][ScanTableError] Exit";
    m_resumeTask.reset();

    return Result::Ok;
}

bool ScanTableErrorBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Resume:
            if (m_resumeInProgress) {
                qDebug() << "[Behavior][ScanTableError] Resume already in progress, ignoring";

                return true;
            }
            qDebug() << "[Behavior][ScanTableError] Resume — starting recovery";
            log("Resuming scan", {"ScanTableError"});
            m_resumeInProgress = true;
            m_resumeTask = runResume();

            return true;

        case Action::Type::Abort:
            cancel();

            return true;
    }

    return AbstractStateBehavior::doAction(action);
}

void ScanTableErrorBehavior::cancel()
{
    qDebug() << "[Behavior][ScanTableError] Cancel — aborting scan";
    log("Scan aborted by user", {"ScanTableError"});

    emit stateEvent("scanFailed", {{"reason", m_reason}});

    if (m_communicator->machineState() == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior(m_alarmCode));
    } else {
        emit transition(this, new IdleBehavior());
    }
}

QCoro::Task<void> ScanTableErrorBehavior::runResume()
{
    // If we came in on an alarm, send $X to unlock and wait (briefly) for Idle.
    if (m_alarmCode != 0) {
        log("Unlocking ($X)", {"ScanTableError"});
        auto r = co_await sendAndAwait("$X", 500ms);
        if (!r) {
            log("Unlock command timed out — stay paused, retry or cancel", {"ScanTableError", "Error"});
            m_resumeInProgress = false;
            co_return;
        }

        auto idle = co_await awaitMachineState(
            [](MachineState s) { return s == MachineState::Idle; },
            1s
        );
        if (!idle) {
            log("Machine did not reach Idle within 1s — stay paused, retry or cancel",
                {"ScanTableError", "Error"});
            m_resumeInProgress = false;
            co_return;
        }

        log("Unlock succeeded, resuming scan", {"ScanTableError"});
    } else if (m_communicator->machineState() != MachineState::Idle) {
        // Non-alarm failure (e.g. GoTo timeout). Require Idle before resuming.
        log(QString("Machine not Idle (%1) — cannot resume yet")
                .arg(static_cast<int>(m_communicator->machineState())),
            {"ScanTableError", "Error"});
        m_resumeInProgress = false;
        co_return;
    }

    emit resumePrevious();
}
