// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "scantablebehavior.h"
#include "gotobehavior.h"
#include "probingbehavior.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"
#include "userpromptbehavior.h"
#include "core/communicator/communicator.h"
#include <cmath>

using namespace std::chrono_literals;

ScanTableBehavior::ScanTableBehavior(Heightmap *heightmap, QPointF startPos, Heightmap::ScanMode scanMode, int moveFeedRate, int probeFeed, QObject *parent)
    : AbstractStateBehavior{parent}
    , m_heightmap(heightmap)
    , m_startPos(startPos)
    , m_scanMode(scanMode)
    , m_moveFeedRate(moveFeedRate)
    , m_probeFeed(probeFeed)
{}

QString ScanTableBehavior::description()
{
    if (!m_grid.isEmpty()) {
        return QString("Scanning table (%1/%2)").arg(m_scannedPoints).arg(m_grid.size());
    }

    return "Scanning table";
}

AbstractStateBehavior::Result ScanTableBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    m_communicator->startQueryingMachineState();

    // Resumed from a UserPromptBehavior raised earlier by raiseScanErrorPrompt.
    if (ctx.previousType == Type::UserPrompt) {
        const QString choiceId = ctx.data.value("choiceId").toString();
        const QVariantMap promptCtx = ctx.data.value("promptContext").toMap();
        const int alarmCode = promptCtx.value("alarmCode").toInt();

        qDebug() << "[Behavior][ScanTable] Resumed from UserPrompt: choice=" << choiceId
                 << "stage:" << (m_phase == Stage::Probing ? "Probing" : "MovingToPoint")
                 << "alarmCode:" << alarmCode;

        if (choiceId == "abort") {
            finishScanning(false, "User cancelled after error");

            return Result::Ok;
        }

        // "resume" — kick off the recovery coroutine. It will unlock the
        // controller when needed and then re-enter processCurrentPoint() /
        // startProbeAtCurrentPoint() based on m_phase.
        log("Resuming scan after user confirmation", {"ScanTable"});
        m_recoveryTask = recoverFromError(alarmCode);

        return Result::Ok;
    }

    if (m_phase == Stage::Initial) {
        m_grid = m_heightmap->probePoints(m_startPos, m_scanMode);
        m_currentPoint = 0;
        m_scannedPoints = 0;

        if (m_grid.isEmpty()) {
            qWarning() << "[Behavior][ScanTable] Scan grid has no points";
            log("Scan grid has no points", {"ScanTable", "Error"});
            finishScanning(false, "Empty grid");
            return Result::Ok;
        }

        qDebug() << "[Behavior][ScanTable] Starting scan:"
                 << m_heightmap->gridWidth() << "x" << m_heightmap->gridHeight()
                 << "=" << m_grid.size() << "points";
        log(QString("Starting table scan: %1 x %2 = %3 points")
                .arg(m_heightmap->gridWidth())
                .arg(m_heightmap->gridHeight())
                .arg(m_grid.size()), {"ScanTable"});

        processCurrentPoint();

    } else if (m_phase == Stage::MovingToPoint) {
        // ── Returned from GoToBehavior ──────────────────────────────────────

        // GoTo delegated an alarm — suspend into a UserPrompt. User decides
        // whether to Resume (which unlocks and retries) or Cancel.
        if (ctx.data.value("alarmOccurred").toBool()) {
            int code = ctx.data.value("alarmCode").toInt();
            QPointF pt = m_grid[m_currentPoint];
            qWarning() << "[Behavior][ScanTable] Alarm" << code << "while moving to point" << pt;
            log(QString("Alarm %1 during move").arg(code), {"ScanTable", "Error"});
            raiseScanErrorPrompt(QString("Alarm %1 during move").arg(code), "move", pt, code);

            return Result::Ok;
        }

        if (!ctx.data.value("success").toBool()) {
            QPointF pt = m_grid[m_currentPoint];
            QString reason = ctx.data.value("failureReason").toString();
            if (reason.isEmpty()) {
                reason = "unknown failure";
            }
            qWarning() << "[Behavior][ScanTable] Move to point" << pt << "failed:" << reason;
            log(QString("Move to point (%1,%2) failed: %3")
                    .arg(pt.x(), 0, 'f', 3).arg(pt.y(), 0, 'f', 3).arg(reason), {"ScanTable", "Error"});
            raiseScanErrorPrompt(reason, "move", pt, /*alarmCode=*/0);

            return Result::Ok;
        }

        startProbeAtCurrentPoint();

    } else if (m_phase == Stage::Probing) {
        // ── Returned from ProbingBehavior ───────────────────────────────────

        // Probe delegated an alarm — suspend into a UserPrompt so user can
        // decide: Resume (unlock + retract + re-probe) or Cancel.
        if (ctx.data.value("alarmOccurred").toBool()) {
            int code = ctx.data.value("alarmCode").toInt();
            QPointF pt = m_grid[m_currentPoint];
            qWarning() << "[Behavior][ScanTable] Alarm" << code << "during probe at point" << m_currentPoint;
            log(QString("Alarm %1 during probe").arg(code), {"ScanTable", "Error"});
            raiseScanErrorPrompt(QString("Alarm %1 during probe").arg(code), "probe", pt, code);

            return Result::Ok;
        }

        QPointF pt = m_grid[m_currentPoint];
        auto [ix, iy] = m_heightmap->gridIndices(pt);

        if (ctx.data.value("success").toBool()) {
            double z = ctx.data.value("z").toDouble();
            m_heightmap->setHeightAt(QPoint(ix, iy), z);
            qDebug() << "[Behavior][ScanTable] Point" << ix << "," << iy << "Z=" << z;
            log(QString("Point (%1,%2): Z=%3").arg(ix).arg(iy).arg(z, 0, 'f', 3), {"ScanTable"});
            emit stateEvent("pointScanned", {{"x", ix}, {"y", iy}, {"z", z}});
        } else {
            qDebug() << "[Behavior][ScanTable] Point" << ix << "," << iy << "probe missed, skipping";
            log(QString("Point (%1,%2): probe missed, skipping").arg(ix).arg(iy), {"ScanTable"});
        }

        m_scannedPoints++;
        emit progressChanged(m_scannedPoints, m_grid.size());  // base class signal

        m_currentPoint++;
        if (m_currentPoint < m_grid.size()) {
            processCurrentPoint();
        } else {
            finishScanning(true);
        }
    }

    return Result::Ok;
}

AbstractStateBehavior::Result ScanTableBehavior::doOnExit(AbstractStateBehavior *next)
{
    qDebug() << "[Behavior][ScanTable] Exit, scanned" << m_scannedPoints << "/" << m_grid.size() << "points";

    return Result::Ok;
}

void ScanTableBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][ScanTable] Alarm" << code << "during scan";
    log(QString("Alarm %1 during scan").arg(code), {"ScanTable", "Error"});
    emit stateEvent("scanFailed", {{"reason", QString("Alarm %1").arg(code)}});
    emit transition(this, new AlarmBehavior(code));
}

void ScanTableBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Alarm) {
        int code = m_communicator->lastAlarmCode();
        qDebug() << "[Behavior][ScanTable] Machine alarm during scan, code" << code;
        log("Machine alarm during scan", {"ScanTable", "Error"});
        emit stateEvent("scanFailed", {{"reason", "Machine alarm"}});
        emit transition(this, new AlarmBehavior(code));
    }
}

// ── Private helpers ──────────────────────────────────────────────────────────

void ScanTableBehavior::processCurrentPoint()
{
    QPointF pos = m_grid[m_currentPoint];
    qDebug() << "[Behavior][ScanTable] Moving to point"
             << (m_currentPoint + 1) << "/" << m_grid.size()
             << "at X=" << pos.x() << "Y=" << pos.y();

    m_phase = Stage::MovingToPoint;
    emit transition(this, new GoToBehavior(pos, m_moveFeedRate, /*delegateAlarmToParent=*/true),
                    TransitionKind::Suspend);
}

void ScanTableBehavior::startProbeAtCurrentPoint()
{
    QPointF pos = m_grid[m_currentPoint];
    qDebug() << "[Behavior][ScanTable] Probing at X=" << pos.x() << "Y=" << pos.y();

    ProbingBehavior::ProbeParameters params;
    auto bt = m_heightmap->zBottomTop();

    params.fastFeedRate = m_probeFeed;
    params.slowFeedRate = std::max(10.0, m_probeFeed / 4.0);
    params.maxDistance = (std::isnan(bt.bottom) || std::isnan(bt.top))
                                 ? 10.0
                                 : std::abs(bt.top - bt.bottom) + 2.0;
    params.retractDistance = 2.0;
    params.safeDistance = 3.0;
    params.setZeroAtProbe = false;
    params.useAbsolute = false;
    params.doubleProbe = true;
    params.delegateAlarmToParent = true;
    params.retractFirst = m_retractBeforeNextProbe;
    m_retractBeforeNextProbe = false;

    m_phase = Stage::Probing;
    emit transition(this, new ProbingBehavior(params), TransitionKind::Suspend);
}

void ScanTableBehavior::finishScanning(bool success, const QString &reason)
{
    if (success) {
        qDebug() << "[Behavior][ScanTable] Scan completed:" << m_scannedPoints << "/" << m_grid.size() << "points";
        log(QString("Table scan completed: %1/%2 points measured")
                .arg(m_scannedPoints).arg(m_grid.size()), {"ScanTable"});
        emit stateEvent("scanCompleted", {});

        emit transition(this, new IdleBehavior());

        return;
    }

    qWarning() << "[Behavior][ScanTable] Scan aborted:" << reason;
    log(QString("Table scan aborted: %1").arg(reason), {"ScanTable", "Error"});
    emit stateEvent("scanFailed", {{"reason", reason}});

    // The failure may have been caused by an unlatched controller alarm
    // (probe-fail, hard limit, etc.). Going straight to Idle would hide that —
    // route into AlarmBehavior so the user has to acknowledge / unlock.
    if (m_communicator->machineState() == MachineState::Alarm) {
        const int code = m_communicator->lastAlarmCode();
        qDebug() << "[Behavior][ScanTable] Machine is in Alarm (code" << code
                 << ") — routing to AlarmBehavior instead of Idle";
        emit transition(this, new AlarmBehavior(code));

        return;
    }

    emit transition(this, new IdleBehavior());
}

void ScanTableBehavior::raiseScanErrorPrompt(const QString &reason, const QString &stage,
                                              std::optional<QPointF> failedPoint, int alarmCode)
{
    PromptSpec spec;
    spec.promptId = QString("scan.%1-failed").arg(stage);
    spec.title = QString("Scan paused — %1").arg(stage);
    spec.message = reason;
    spec.context = {
        {"stage", stage},
        {"alarmCode", alarmCode},
        {"reason", reason},
        {"point", failedPoint.value_or(QPointF(0, 0))},
    };
    spec.choices = {
        {"resume", "Resume", /*destructive=*/false, /*isDefault=*/true},
        {"abort", "Abort", /*destructive=*/true, /*isDefault=*/false},
    };

    emit stateEvent("scanPaused", {
        {"reason", reason},
        {"stage", stage},
        {"alarmCode", alarmCode},
    });

    emit transition(this, new UserPromptBehavior(spec), TransitionKind::Suspend);
}

QCoro::Task<void> ScanTableBehavior::recoverFromError(int alarmCode)
{
    if (alarmCode != 0) {
        log("Unlocking ($X)", {"ScanTable"});
        auto r = co_await sendAndAwait("$X", 500ms);
        if (!r) {
            log("Unlock command timed out — re-prompting", {"ScanTable", "Error"});
            raiseScanErrorPrompt("Unlock timed out", m_phase == Stage::Probing ? "probe" : "move",
                                 std::nullopt, alarmCode);

            co_return;
        }
        auto idle = co_await awaitMachineState(
            [](MachineState s) { return s == MachineState::Idle; },
            1s
        );
        if (!idle) {
            log("Machine did not reach Idle within 1s — re-prompting",
                {"ScanTable", "Error"});
            raiseScanErrorPrompt("Machine did not become Idle",
                                 m_phase == Stage::Probing ? "probe" : "move",
                                 std::nullopt, alarmCode);

            co_return;
        }
        log("Unlock succeeded", {"ScanTable"});
    } else if (m_communicator->machineState() != MachineState::Idle) {
        log(QString("Machine not Idle (%1) — re-prompting")
                .arg(static_cast<int>(m_communicator->machineState())),
            {"ScanTable", "Error"});
        raiseScanErrorPrompt("Machine not Idle",
                             m_phase == Stage::Probing ? "probe" : "move",
                             std::nullopt, /*alarmCode=*/0);

        co_return;
    }

    if (m_phase == Stage::Probing) {
        m_retractBeforeNextProbe = true;
        startProbeAtCurrentPoint();
    } else {
        processCurrentPoint();
    }
}
