// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef C2C1B619_D54C_4260_9997_6906E6DBB180
#define C2C1B619_D54C_4260_9997_6906E6DBB180
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PROBINGBEHAVIOR_H
#define PROBINGBEHAVIOR_H

#include "statebehavior.h"
#include <QVector3D>

class ProbingBehavior : public StateBehavior
{
    public:
        // Probe direction
        enum class ProbeDirection {
            ZMinus,     // Probe downward (most common)
            ZPlus,      // Probe upward
            XMinus,     // Probe left
            XPlus,      // Probe right
            YMinus,     // Probe backward
            YPlus       // Probe forward
        };

        explicit ProbingBehavior(ProbeDirection direction = ProbeDirection::ZMinus,
                               double distance = 20.0,
                               double feedRate = 100.0,
                               QObject *parent = nullptr);
        QString name() override { return "Probing"; }
        bool isJoggingAllowed() override { return false; } // Cannot jog during probing
        bool isHomingAllowed() override { return false; } // Cannot home during probing

        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, QString response, QStringList fullResponse) override;

        // Probing-specific methods
        void setProbeParameters(ProbeDirection direction, double distance, double feedRate);
        QVector3D getProbeResult() const { return m_probeResult; }

    private:
        ProbeDirection m_direction;
        double m_distance;
        double m_feedRate;
        bool m_probeStarted;
        bool m_probeCompleted;
        QVector3D m_probeResult; // Coordinates where the probe touched the surface

        // Helper method to generate G38.2 command based on parameters
        QString generateProbeCommand();
};

#endif // PROBINGBEHAVIOR_H


#endif /* C2C1B619_D54C_4260_9997_6906E6DBB180 */
