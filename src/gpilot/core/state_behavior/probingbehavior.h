#ifndef PROBINGBEHAVIOR_H
#define PROBINGBEHAVIOR_H

#include "statebehavior.h"
#include <QVector3D>

class ProbingBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        enum class ProbeStage {
            InitialSetup,           // Setup initial state (G91, units)
            FastProbe,              // First probe - fast to find surface
            FastProbeWait,          // Waiting for fast probe result
            Retract,                // Retract after fast probe
            RetractWait,            // Waiting for retract
            SlowProbe,              // Second probe - slow for precision
            SlowProbeWait,          // Waiting for slow probe result
            SetZero,                // Set Z=0 at probe position (optional)
            MoveToSafe,             // Move to safe position
            Completed               // All done
        };

        struct ProbeParameters {
            double fastFeedRate = 200.0;     // Fast probe speed (mm/min)
            double slowFeedRate = 50.0;      // Slow probe speed (mm/min)
            double maxDistance = 30.0;       // Maximum probe distance (mm)
            double retractDistance = 2.0;    // Retract distance between probes (mm)
            double safeDistance = 5.0;       // Safe distance to move up after probing (mm)
            bool doubleProbe = false;        // Whether to do a second slow probe for precision
            bool setZeroAtProbe = false;     // Set Z=0 at probed position
            bool useAbsolute = false;        // Return to absolute positioning
        };

        explicit ProbingBehavior(QObject* parent = nullptr);
        explicit ProbingBehavior(ProbeParameters params, QObject* parent = nullptr);
        QString description() override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onAlarm(int code) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        void onMachineStateChanged(MachineState state) override;

        QVector3D probedPosition() const { return m_probedPosition; }
        bool wasSuccessful() const { return m_success; }

    protected:
        QString name() const override { return "ProbingBehavior"; }

    private:
        ProbeParameters m_params;
        ProbeStage m_stage;
        bool m_alarmOccurred;
        int m_alarmCode;
        QVector3D m_fastProbePosition;
        QVector3D m_probedPosition;  // Final precise position
        bool m_success;
        bool m_initialStateAbsolute;  // Remember if we started in absolute mode

        void startFastProbe();
        void startRetract();
        void startSlowProbe();
        void setZeroPosition();
        void moveToSafePosition();
        bool parseProbeResponse(const QStringList &fullResponse, QVector3D &position, bool &contacted);
        void finishProbing(bool success);
        QString stageDescription() const;

    signals:
        void probeCompleted(QVector3D position);
        void probeFailed(QString reason);
};

#endif // PROBINGBEHAVIOR_H
