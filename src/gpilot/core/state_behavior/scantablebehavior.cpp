// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "scantablebehavior.h"
#include "gotobehavior.h"
#include "probingbehavior.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"
#include "core/communicator/communicator.h"
#include <cmath>

ScanTableBehavior::ScanTableBehavior(Heightmap *heightmap, QPointF startPos, Heightmap::ScanMode scanMode, int moveFeedRate, QObject *parent)
    : StateBehavior{parent}
    , m_heightmap(heightmap)
    , m_startPos(startPos)
    , m_scanMode(scanMode)
    , m_moveFeedRate(moveFeedRate)
{}

QString ScanTableBehavior::description()
{
    if (!m_grid.isEmpty()) {
        return QString("Scanning table (%1/%2)").arg(m_scannedPoints).arg(m_grid.size());
    }

    return "Scanning table";
}

StateBehavior::Result ScanTableBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    if (m_phase == Phase::Initial) {
        m_grid = m_heightmap->probePoints(m_startPos, m_scanMode);
        m_currentPoint = 0;
        m_scannedPoints = 0;

        if (m_grid.isEmpty()) {
            log("Scan grid has no points", {"ScanTable", "Error"});
            finishScanning(false, "Empty grid");
            return Result::Ok;
        }

        m_communicator->startQueryingMachineState();

        log(QString("Starting table scan: %1 x %2 = %3 points")
                .arg(m_heightmap->gridWidth())
                .arg(m_heightmap->gridHeight())
                .arg(m_grid.size()), {"ScanTable"});

        processCurrentPoint();

    } else if (m_phase == Phase::MovingToPoint) {
        // ── Returned from GoToBehavior ──────────────────────────────────────
        // GoToBehavior waits for Idle before returning, so a non-Idle state
        // here means the move command failed (e.g. soft-limit error).
        if (m_communicator->machineState() != MachineState::Idle) {
            QPointF pt = m_grid[m_currentPoint];
            log(QString("Move to point (%1,%2) failed, aborting scan")
                    .arg(pt.x(), 0, 'f', 3).arg(pt.y(), 0, 'f', 3), {"ScanTable", "Error"});
            finishScanning(false, QString("Failed to move to (X%1 Y%2)")
                               .arg(pt.x(), 0, 'f', 3).arg(pt.y(), 0, 'f', 3));
            return Result::Ok;
        }

        startProbeAtCurrentPoint();

    } else if (m_phase == Phase::Probing) {
        // ── Returned from ProbingBehavior ───────────────────────────────────
        auto *prob = qobject_cast<ProbingBehavior*>(previous);
        QPointF pt = m_grid[m_currentPoint];
        auto [ix, iy] = m_heightmap->gridIndices(pt);

        if (prob && prob->wasSuccessful()) {
            double z = prob->probedPosition().z();
            m_heightmap->setHeightAt(QPoint(ix, iy), z);
            log(QString("Point (%1,%2): Z=%3").arg(ix).arg(iy).arg(z, 0, 'f', 3), {"ScanTable"});
            emit pointScanned(ix, iy, z);
        } else {
            log(QString("Point (%1,%2): probe missed, skipping").arg(ix).arg(iy), {"ScanTable"});
        }

        m_scannedPoints++;
        emit progressChanged(m_scannedPoints, m_grid.size());

        m_currentPoint++;
        if (m_currentPoint < m_grid.size()) {
            processCurrentPoint();
        } else {
            finishScanning(true);
        }
    }

    return Result::Ok;
}

StateBehavior::Result ScanTableBehavior::onExit(StateBehavior *next)
{
    return StateBehavior::onExit(next);
}

void ScanTableBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][ScanTable] Alarm" << code << "during scan";
    log(QString("Alarm %1 during scan").arg(code), {"ScanTable", "Error"});
    emit scanFailed(QString("Alarm %1").arg(code));
    emit transition(this, new AlarmBehavior(code));
}

void ScanTableBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Alarm) {
        int code = m_communicator->lastAlarmCode();
        qDebug() << "[Behavior][ScanTable] Machine alarm during scan, code" << code;
        log("Machine alarm during scan", {"ScanTable", "Error"});
        emit scanFailed("Machine alarm");
        emit transition(this, new AlarmBehavior(code));
    }
}

// ── Private helpers ──────────────────────────────────────────────────────────

void ScanTableBehavior::processCurrentPoint()
{
    QPointF pos = m_grid[m_currentPoint];
    log(QString("Moving to point %1/%2 at X=%3 Y=%4")
            .arg(m_currentPoint + 1).arg(m_grid.size())
            .arg(pos.x(), 0, 'f', 3)
            .arg(pos.y(), 0, 'f', 3), {"ScanTable"});

    m_phase = Phase::MovingToPoint;
    emit transition(this, new GoToBehavior(pos, m_moveFeedRate));
}

void ScanTableBehavior::startProbeAtCurrentPoint()
{
    QPointF pos = m_grid[m_currentPoint];
    log(QString("Probing at X=%1 Y=%2").arg(pos.x(), 0, 'f', 3).arg(pos.y(), 0, 'f', 3), {"ScanTable"});

    ProbingBehavior::ProbeParameters params;
    auto bt = m_heightmap->zBottomTop();

    params.fastFeedRate = m_heightmap->probeFeed();
    params.slowFeedRate = std::max(10.0, m_heightmap->probeFeed() / 4.0);
    params.maxDistance = (std::isnan(bt.bottom) || std::isnan(bt.top))
                                 ? 20.0
                                 : std::abs(bt.top - bt.bottom) + 2.0;
    params.retractDistance = 2.0;
    params.safeDistance = 3.0;
    params.setZeroAtProbe = false;
    params.useAbsolute = false;

    m_phase = Phase::Probing;
    emit transition(this, new ProbingBehavior(params));
}

void ScanTableBehavior::finishScanning(bool success, const QString &reason)
{
    if (success) {
        log(QString("Table scan completed: %1/%2 points measured")
                .arg(m_scannedPoints).arg(m_grid.size()), {"ScanTable"});
        emit scanCompleted();
    } else {
        log(QString("Table scan aborted: %1").arg(reason), {"ScanTable", "Error"});
        emit scanFailed(reason);
    }

    emit transition(this, new IdleBehavior());
}
