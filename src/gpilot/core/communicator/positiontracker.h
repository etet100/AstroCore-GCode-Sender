#pragma once

#include <QObject>
#include <QVector3D>
#include "machinecoordinatecache.h"
#include "core/globals.h"

class Communicator;
class PhysicalMachineConfiguration;

// Tracks machine and work positions reported by GRBL status messages.
// Owns a MachineCoordinateCache (M, W, and offset vars from $#).
class PositionTracker : public QObject
{
    Q_OBJECT

public:
    explicit PositionTracker(Communicator* communicator);

    QVector3D machinePos() const { return m_machinePos; }
    QVector3D workOffset() const { return m_workOffset; }
    QVector3D workPos() const { return m_machinePos - m_workOffset; }
    MachineCoordinateCache& coordinateCache() { return m_coordCache; }

    // Resets positions to zero (called on connection reset).
    void reset();

    bool compareCoordinates(double x, double y, double z) const;

    // Sends G53/G92 commands to restore pre-reset position.
    void restoreOffsets(PhysicalMachineConfiguration* config);

    // Called from processStatus for each status report section.
    void processMachinePosition(const QString& line);
    void processWorkPosition(const QString& line);
    void processWorkOffset(const QString& line);

    // Called after all status sections are processed.
    void processNewToolPosition(bool isCheckMode, bool isLastCommandProcessed);

    // Called when $# response arrives.
    void processOffsetsVars(const QStringList& response);

signals:
    void machinePosChanged(QVector3D pos);
    void toolPositionReceived(QVector3D pos);

private:
    Communicator* m_communicator;
    QVector3D m_machinePos;
    QVector3D m_workOffset;
    MachineCoordinateCache m_coordCache;
};
