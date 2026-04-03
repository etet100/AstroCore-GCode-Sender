#ifndef SCANTABLEBEHAVIOR_H
#define SCANTABLEBEHAVIOR_H

#include "statebehavior.h"
#include "core/heightmap/heightmap.h"

// ScanTableBehavior orchestrates an automatic height-map scan.
//
// Flow for each grid point:
//   1. GoToBehavior  — jog to the X,Y position above the point
//   2. ProbingBehavior — probe downward and record the Z contact position
//
// The behavior re-enters itself after each sub-state (GoTo / Probing)
// returns via transitionToPreviousState().  m_phase tracks which stage
// we are at so onEntry() knows what to do on each re-entry.
//
// Scan order is determined by Heightmap::probePoints() — serpentine
// starting from the grid vertex nearest to the machine's current position.
class ScanTableBehavior : public StateBehavior
{
    Q_OBJECT

public:
    explicit ScanTableBehavior(
        Heightmap *heightmap,
        QPointF startPos,
        Heightmap::ScanMode scanMode = Heightmap::ScanMode::Rows,
        int moveFeedRate = 1000,
        QObject *parent = nullptr
    );

    QString description() override;
    Type type() const override { return Type::ScanTable; }

    Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
    Result onExit(StateBehavior *next = nullptr) override;
    void onAlarm(int code) override;
    void onMachineStateChanged(MachineState state) override;

protected:
    QString name() const override { return "ScanTable"; }

private:
    enum class Stage {
        Initial,        // not yet started
        MovingToPoint,  // GoToBehavior is active
        Probing         // ProbingBehavior is active
    };

    Heightmap           *m_heightmap;
    QPointF              m_startPos;
    Heightmap::ScanMode  m_scanMode;
    int                  m_moveFeedRate;

    QList<QPointF>  m_grid;
    int             m_currentPoint = 0;
    int             m_scannedPoints = 0;

    Stage m_phase = Stage::Initial;

    void processCurrentPoint();
    void startProbeAtCurrentPoint();
    void finishScanning(bool success, const QString &reason = QString());
};

#endif // SCANTABLEBEHAVIOR_H
