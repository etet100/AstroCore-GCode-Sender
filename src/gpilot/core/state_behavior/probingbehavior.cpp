// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "probingbehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"
#include <QCoroTimer>

using namespace std::chrono_literals;

ProbingBehavior::ProbingBehavior(QObject* parent)
    : AbstractStateBehavior{parent}
{}

ProbingBehavior::ProbingBehavior(ProbeParameters params, QObject* parent)
    : AbstractStateBehavior{parent}
    , m_params(params)
{}

QString ProbingBehavior::description()
{
    return QString("Probing - %1").arg(m_stageDescription);
}

AbstractStateBehavior::Result ProbingBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Probing] Entry - starting probing sequence"
             << (m_params.doubleProbe ? "(two-phase)" : "");

    setExitValue("success", false);

    m_communicator->startQueryingMachineState();
    m_probingTask = runProbingSequence();

    return AbstractStateBehavior::Result::Ok;
}

AbstractStateBehavior::Result ProbingBehavior::doOnExit(AbstractStateBehavior *next)
{
    qDebug() << "[Behavior][Probing] Exit";

    m_probingTask.reset();
    m_communicator->stopQueryingMachineState();

    return Result::Ok;
}

void ProbingBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][Probing] Alarm during probing:" << code;

    m_alarmOccurred = true;
    m_alarmCode = code;

    if (m_params.delegateAlarmToParent) {
        // Wake any coroutine currently inside awaitMachineState so it can
        // observe m_alarmOccurred and emit the proper delegation exit
        // without waiting for the full await timeout.
        emit machineStateSignal(MachineState::Alarm);
    } else {
        emit transition(this, new AlarmBehavior(code));
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void ProbingBehavior::emitAlarmExit()
{
    if (m_params.delegateAlarmToParent) {
        setExitValue({
            {"alarmOccurred", true},
            {"alarmCode", m_alarmCode},
        });
        emit resumePrevious();
    } else {
        emit transition(this, new AlarmBehavior(m_alarmCode));
    }
}

QCoro::Task<void> ProbingBehavior::emitFailureExit()
{
    if (m_params.useAbsolute) {
        co_await sendAndAwait("G90", m_params.setupTimeout);
    }
    emit resumePrevious();
}

QCoro::Task<bool> ProbingBehavior::waitForMotionComplete(const QString &stage)
{
    qDebug().noquote() << QString("[Behavior][Probing][%1] Waiting for Run").arg(stage);
    auto moving = co_await awaitMachineState(
        [](MachineState s) { return s == MachineState::Run; },
        500ms
    );

    if (m_alarmOccurred) {
        log(QString("Alarm while waiting for Run after %1: %2").arg(stage).arg(m_alarmCode), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", QString("Alarm %1 after %2").arg(m_alarmCode).arg(stage)}});
        emitAlarmExit();
        co_return false;
    }

    if (!moving) {
        qDebug().noquote() << QString("[Behavior][Probing][%1] No Run observed within 500ms, assuming already idle").arg(stage);
        co_return true;
    }

    qDebug().noquote() << QString("[Behavior][Probing][%1] Run observed, now waiting for Idle").arg(stage);
    auto idle = co_await awaitMachineState(
        [](MachineState s) { return s == MachineState::Idle; },
        m_params.moveTimeout
    );

    if (m_alarmOccurred) {
        log(QString("Alarm while waiting for Idle after %1: %2").arg(stage).arg(m_alarmCode), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", QString("Alarm %1 after %2").arg(m_alarmCode).arg(stage)}});
        emitAlarmExit();
        co_return false;
    }

    if (!idle) {
        log(QString("Timeout waiting for Idle after %1").arg(stage), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", QString("Timeout waiting for Idle after %1").arg(stage)}});
        co_await emitFailureExit();
        co_return false;
    }

    qDebug().noquote() << QString("[Behavior][Probing][%1] Idle observed").arg(stage);
    co_return true;
}

QCoro::Task<std::optional<QVector3D>> ProbingBehavior::executeProbe(
    const QString &stage, double distance, double feedRate)
{
    QString cmd = QString("G38.2 Z-%1 F%2")
        .arg(distance, 0, 'f', 3)
        .arg(feedRate, 0, 'f', 1);
    log(QString("%1: %2").arg(stage, cmd), {"Probing"});

    // A probe can finish in four ways:
    //   1. command rejected (!r->status.ok) — nothing started
    //   2. alarm first, response follows with contacted=false
    //   3. response first with contacted=false, alarm follows shortly after
    //   4. response with contacted=true — success
    // We must have BOTH the command response and the alarm flag settled before
    // classifying the outcome, otherwise a late alarm leaks into the next stage
    // (or ScanTable) and gets misreported.
    auto r = co_await sendAndAwait(cmd, m_params.probeTimeout);

    if (!r) {
        log(QString("Timeout during %1").arg(stage), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", QString("Timeout during %1").arg(stage)}});
        co_await emitFailureExit();
        co_return std::nullopt;
    }
    if (!r->status.ok) {
        log(QString("%1 error: %2").arg(stage, enrichErrorMessage(r->response)), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", QString("%1 command failed").arg(stage)}});
        co_await emitFailureExit();
        co_return std::nullopt;
    }

    auto probe = ProbeResponseParser::parse(r->fullResponse);
    bool contacted = probe && probe->contacted;

    // Grace period: if the response says "no contact" but no alarm has been
    // observed yet, wait briefly for one to arrive — PROBE_FAIL often follows
    // the response by a few ms. Bail out early on Alarm or Idle (steady states).
    if (!contacted && !m_alarmOccurred) {
        co_await awaitMachineState(
            [](MachineState s) { return s == MachineState::Alarm || s == MachineState::Idle; },
            500ms
        );
    }

    if (m_alarmOccurred) {
        if (m_alarmCode == GRBL_ALARM_PROBE_FAIL_1 || m_alarmCode == GRBL_ALARM_PROBE_FAIL_2) {
            log(QString("%1 failed - no contact (alarm %2)").arg(stage).arg(m_alarmCode), {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", QString("No contact during %1").arg(stage)}});
        } else {
            log(QString("Alarm during %1: %2").arg(stage).arg(m_alarmCode), {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", QString("Alarm %1").arg(m_alarmCode)}});
        }
        emitAlarmExit();
        co_return std::nullopt;
    }

    if (!contacted) {
        log(QString("%1 - no contact (no alarm)").arg(stage), {"Probing", "Error"});
        qDebug() << "[Behavior][Probing] No contact";
        emit stateEvent("probeFailed", {{"reason", QString("No contact during %1").arg(stage)}});
        co_await emitFailureExit();
        co_return std::nullopt;
    }

    qDebug() << "[Behavior][Probing][Contact]" << stage << "Z=" << probe->position.z();
    log(QString("%1 contact at Z=%2").arg(stage).arg(probe->position.z(), 0, 'f', 3), {"Probing"});

    co_return probe->position;
}

// ---------------------------------------------------------------------------
// Main probing coroutine — single top-to-bottom flow
// ---------------------------------------------------------------------------

QCoro::Task<void> ProbingBehavior::runProbingSequence()
{
    log("Starting probing sequence...", {"Probing"});

    // ── Step 1: Switch to relative positioning + metric ──────────────
    m_stageDescription = "Setup";
    auto r = co_await sendAndAwait("G91 G21", m_params.setupTimeout);

    if (!r) {
        log("Timeout during initial setup", {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Timeout during setup"}});
        emit resumePrevious();
        co_return;
    }
    if (m_alarmOccurred) {
        log(QString("Alarm during setup: %1").arg(m_alarmCode), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", QString("Alarm %1 during setup").arg(m_alarmCode)}});
        emitAlarmExit();
        co_return;
    }
    if (!r->status.ok) {
        log(QString("Setup error: %1").arg(enrichErrorMessage(r->response)), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Setup command failed"}});
        emit resumePrevious();
        co_return;
    }

    // ── Step 1.5: Optional pre-retract (alarm-recovery entry) ───────
    if (m_params.retractFirst) {
        m_stageDescription = "Pre-retract";
        QString cmd = QString("G0 Z%1").arg(m_params.safeDistance, 0, 'f', 3);
        log(QString("Pre-retract: %1mm").arg(m_params.safeDistance, 0, 'f', 3), {"Probing"});

        r = co_await sendAndAwait(cmd, m_params.moveTimeout);

        if (!r || m_alarmOccurred || !r->status.ok) {
            log("Error during pre-retract", {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", "Pre-retract failed"}});
            if (m_alarmOccurred) {
                emitAlarmExit();
            } else {
                co_await emitFailureExit();
            }

            co_return;
        }

        if (!co_await waitForMotionComplete("pre-retract")) {
            co_return;
        }
    }

    // ── Step 2: Fast probe ───────────────────────────────────────────
    m_stageDescription = "Fast Probe";
    auto fastPos = co_await executeProbe("fast probe", m_params.maxDistance, m_params.fastFeedRate);
    if (!fastPos) {
        co_return;
    }
    QVector3D fastProbePosition = *fastPos;

    if (!co_await waitForMotionComplete("fast probe")) {
        co_return;
    }

    // ── Step 3: Retract ──────────────────────────────────────────────
    m_stageDescription = "Retract";
    QString retractCmd = QString("G0 Z%1").arg(m_params.retractDistance, 0, 'f', 3);
    qDebug() << "[Behavior][Probing] Retracting";
    log(QString("Retracting: %1mm").arg(m_params.retractDistance, 0, 'f', 3), {"Probing"});

    r = co_await sendAndAwait(retractCmd, m_params.moveTimeout);

    if (!r || m_alarmOccurred || !r->status.ok) {
        log("Error during retract", {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Retract failed"}});
        if (m_alarmOccurred) {
            emitAlarmExit();
        } else {
            co_await emitFailureExit();
        }
        co_return;
    }

    // ── Step 3.1: Wait for retract motion to start, then for Idle ────
    if (!co_await waitForMotionComplete("retract")) {
        co_return;
    }

    // ── Step 4: Slow probe (optional, for precision) ─────────────────
    QVector3D finalPosition = fastProbePosition;

    if (m_params.doubleProbe) {
        m_stageDescription = "Slow Probe";
        auto slowPos = co_await executeProbe(
            "slow probe", m_params.retractDistance + 1.0, m_params.slowFeedRate);
        if (!slowPos) {
            co_return;
        }
        finalPosition = *slowPos;

        if (!co_await waitForMotionComplete("slow probe")) {
            co_return;
        }
    }

    qDebug() << "[Behavior][Probing] Probing completed";

    m_probedPosition = finalPosition;
    m_success = true;
    setExitValue({
        {"success", true},
        {"x", m_probedPosition.x()},
        {"y", m_probedPosition.y()},
        {"z", m_probedPosition.z()},
    });
    emit stateEvent("probeCompleted", {{"x", m_probedPosition.x()}, {"y", m_probedPosition.y()}, {"z", m_probedPosition.z()}});

    // ── Step 5: Set Z=0 at probe position (optional) ────────────────
    if (m_params.setZeroAtProbe) {
        m_stageDescription = "Set Zero";
        log("Setting Z=0 at probe position", {"Probing"});

        r = co_await sendAndAwait("G92 Z0", m_params.setupTimeout);

        if (r && r->status.ok) {
            log("Z axis zeroed at probe position", {"Probing"});
        }
    }

    // ── Step 6: Move to safe height ─────────────────────────────────
    qDebug() << "[Behavior][Probing] Moving to safe height";
    m_stageDescription = "Move to Safe";
    QString safeCmd = QString("G0 Z%1").arg(m_params.safeDistance, 0, 'f', 3);
    log(QString("Moving to safe position: +%1mm").arg(m_params.safeDistance, 0, 'f', 3), {"Probing"});

    co_await sendAndAwait(safeCmd, m_params.moveTimeout);

    // ── Step 6.1: Wait for safe-move motion to start, then for Idle ──
    if (!co_await waitForMotionComplete("safe move")) {
        co_return;
    }

    // ── Step 7: Restore absolute positioning ────────────────────────
    if (m_params.useAbsolute) {
        m_stageDescription = "Restore Mode";
        co_await sendAndAwait("G90", m_params.setupTimeout);
    }

    // ── Done ─────────────────────────────────────────────────────────
    m_stageDescription = "Completed";
    log("Probing completed successfully", {"Probing"});
    emit resumePrevious();
}
