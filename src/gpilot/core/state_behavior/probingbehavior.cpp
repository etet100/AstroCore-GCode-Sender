// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "probingbehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

ProbingBehavior::ProbingBehavior(QObject* parent)
    : StateBehavior{parent}
{}

ProbingBehavior::ProbingBehavior(ProbeParameters params, QObject* parent)
    : StateBehavior{parent}
    , m_params(params)
{}

QString ProbingBehavior::description()
{
    return QString("Probing - %1").arg(m_stageDescription);
}

StateBehavior::Result ProbingBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Probing] Entry - starting probing sequence"
             << (m_params.doubleProbe ? "(two-phase)" : "");

    setExitValue("success", false);

    m_communicator->startQueryingMachineState();
    m_probingTask = runProbingSequence();

    return StateBehavior::Result::Ok;
}

StateBehavior::Result ProbingBehavior::doOnExit(StateBehavior *next)
{
    qDebug() << "[Behavior][Probing] Exit";

    m_probingTask.reset();
    m_communicator->stopQueryingMachineState();

    return Result::Ok;
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
        if (m_params.delegateAlarmToParent) {
            setExitValue("alarmOccurred", true);
            setExitValue("alarmCode", m_alarmCode);
            emit resumePrevious();
        } else {
            emit transition(this, new AlarmBehavior(m_alarmCode));
        }
        co_return;
    }
    if (!r->status.ok) {
        log(QString("Setup error: %1").arg(enrichErrorMessage(r->response)), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Setup command failed"}});
        emit resumePrevious();
        co_return;
    }

    // ── Step 2: Fast probe ───────────────────────────────────────────
    m_stageDescription = "Fast Probe";
    QString fastProbeCmd = QString("G38.2 Z-%1 F%2")
        .arg(m_params.maxDistance, 0, 'f', 3)
        .arg(m_params.fastFeedRate, 0, 'f', 1);
    log(QString("Fast probe: %1").arg(fastProbeCmd), {"Probing"});

    r = co_await sendAndAwait(fastProbeCmd, m_params.probeTimeout);

    if (!r) {
        log("Timeout during fast probe", {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Timeout during fast probe"}});
        if (m_params.useAbsolute) {
            co_await sendAndAwait("G90", m_params.setupTimeout);
        }
        emit resumePrevious();
        co_return;
    }
    if (m_alarmOccurred) {
        if (m_alarmCode == GRBL_ALARM_PROBE_FAIL_1 || m_alarmCode == GRBL_ALARM_PROBE_FAIL_2) {
            log("Probe failed - no contact detected", {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", "No contact detected during probing"}});
        } else {
            log(QString("Alarm during fast probe: %1").arg(m_alarmCode), {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", QString("Alarm %1").arg(m_alarmCode)}});
        }
        if (m_params.delegateAlarmToParent) {
            setExitValue("alarmOccurred", true);
            setExitValue("alarmCode", m_alarmCode);
            emit resumePrevious();
        } else {
            emit transition(this, new AlarmBehavior(m_alarmCode));
        }
        co_return;
    }
    if (!r->status.ok) {
        log(QString("Fast probe error: %1").arg(enrichErrorMessage(r->response)), {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Fast probe command failed"}});
        if (m_params.useAbsolute) {
            co_await sendAndAwait("G90", m_params.setupTimeout);
        }
        emit resumePrevious();
        co_return;
    }

    auto fastProbe = ProbeResponseParser::parse(r->fullResponse);
    if (!fastProbe || !fastProbe->contacted) {
        log("Fast probe - no contact or unparseable response", {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "No contact during fast probe"}});
        if (m_params.useAbsolute) {
            co_await sendAndAwait("G90", m_params.setupTimeout);
        }
        emit resumePrevious();
        co_return;
    }

    log(QString("Fast probe contact at Z=%1").arg(fastProbe->position.z(), 0, 'f', 3), {"Probing"});

    QVector3D fastProbePosition = fastProbe->position;

    // ── Step 3: Retract ──────────────────────────────────────────────
    m_stageDescription = "Retract";
    QString retractCmd = QString("G0 Z%1").arg(m_params.retractDistance, 0, 'f', 3);
    log(QString("Retracting: %1mm").arg(m_params.retractDistance, 0, 'f', 3), {"Probing"});

    r = co_await sendAndAwait(retractCmd, m_params.moveTimeout);

    if (!r || m_alarmOccurred || !r->status.ok) {
        log("Error during retract", {"Probing", "Error"});
        emit stateEvent("probeFailed", {{"reason", "Retract failed"}});
        if (m_alarmOccurred) {
            if (m_params.delegateAlarmToParent) {
                setExitValue("alarmOccurred", true);
                setExitValue("alarmCode", m_alarmCode);
                emit resumePrevious();
            } else {
                emit transition(this, new AlarmBehavior(m_alarmCode));
            }
        } else {
            if (m_params.useAbsolute) {
                co_await sendAndAwait("G90", m_params.setupTimeout);
            }
            emit resumePrevious();
        }
        co_return;
    }

    // ── Step 4: Slow probe (optional, for precision) ─────────────────
    QVector3D finalPosition = fastProbePosition;

    if (m_params.doubleProbe) {
        m_stageDescription = "Slow Probe";
        double slowProbeDistance = m_params.retractDistance + 1.0;
        QString slowProbeCmd = QString("G38.2 Z-%1 F%2")
            .arg(slowProbeDistance, 0, 'f', 3)
            .arg(m_params.slowFeedRate, 0, 'f', 1);
        log(QString("Slow probe: %1").arg(slowProbeCmd), {"Probing"});

        r = co_await sendAndAwait(slowProbeCmd, m_params.probeTimeout);

        if (!r) {
            log("Timeout during slow probe", {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", "Timeout during slow probe"}});
            if (m_params.useAbsolute) {
                co_await sendAndAwait("G90", m_params.setupTimeout);
            }
            emit resumePrevious();
            co_return;
        }
        if (m_alarmOccurred) {
            if (m_alarmCode == GRBL_ALARM_PROBE_FAIL_1 || m_alarmCode == GRBL_ALARM_PROBE_FAIL_2) {
                log("Slow probe failed - no contact detected", {"Probing", "Error"});
                emit stateEvent("probeFailed", {{"reason", "No contact during slow probe"}});
            } else {
                log(QString("Alarm during slow probe: %1").arg(m_alarmCode), {"Probing", "Error"});
                emit stateEvent("probeFailed", {{"reason", QString("Alarm %1").arg(m_alarmCode)}});
            }
            if (m_params.delegateAlarmToParent) {
                setExitValue("alarmOccurred", true);
                setExitValue("alarmCode", m_alarmCode);
                emit resumePrevious();
            } else {
                emit transition(this, new AlarmBehavior(m_alarmCode));
            }
            co_return;
        }
        if (!r->status.ok) {
            log(QString("Slow probe error: %1").arg(enrichErrorMessage(r->response)), {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", "Slow probe command failed"}});
            if (m_params.useAbsolute) {
                co_await sendAndAwait("G90", m_params.setupTimeout);
            }
            emit resumePrevious();
            co_return;
        }

        auto slowProbe = ProbeResponseParser::parse(r->fullResponse);
        if (!slowProbe || !slowProbe->contacted) {
            log("Slow probe - no contact or unparseable response", {"Probing", "Error"});
            emit stateEvent("probeFailed", {{"reason", "No contact during slow probe"}});
            if (m_params.useAbsolute) {
                co_await sendAndAwait("G90", m_params.setupTimeout);
            }
            emit resumePrevious();
            co_return;
        }

        finalPosition = slowProbe->position;
        log(QString("Precise probe contact at Z=%1").arg(slowProbe->position.z(), 0, 'f', 3), {"Probing"});
    }

    m_probedPosition = finalPosition;
    m_success = true;
    setExitValue("success", true);
    setExitValue("x", m_probedPosition.x());
    setExitValue("y", m_probedPosition.y());
    setExitValue("z", m_probedPosition.z());
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
    m_stageDescription = "Move to Safe";
    QString safeCmd = QString("G0 Z%1").arg(m_params.safeDistance, 0, 'f', 3);
    log(QString("Moving to safe position: +%1mm").arg(m_params.safeDistance, 0, 'f', 3), {"Probing"});

    co_await sendAndAwait(safeCmd, m_params.moveTimeout);

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
