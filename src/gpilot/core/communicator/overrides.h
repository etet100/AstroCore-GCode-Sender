#pragma once

#include <QObject>

class Communicator;

// Manages GRBL speed overrides for feed rate, rapid speed, and spindle speed.
// It connects to Communicator and automatically sends realtime commands
// to adjust machine speed toward the user-defined targets.
class Overrides : public QObject
{
    Q_OBJECT

public:
    explicit Overrides(Communicator *communicator);

    // Called by GUI when the user changes target values.
    // Unset overrides should pass 100 as value.
    void setTargets(bool feedOverridden, int feed,
                    bool rapidOverridden, int rapid,
                    bool spindleOverridden, int spindle);

    int targetFeed() const    { return m_targetFeed; }
    int targetSpindle() const { return m_targetSpindle; }
    int targetRapid() const   { return m_targetRapid; }

    bool isFeedOverridden() const    { return m_feedOverridden; }
    bool isRapidOverridden() const   { return m_rapidOverridden; }
    bool isSpindleOverridden() const { return m_spindleOverridden; }

signals:
    // Emitted each time the machine reports its current override values.
    void currentValuesChanged(int feed, int spindle, int rapid);

private slots:
    void onOverridesReceived(int feedOverride, int spindleOverride, int rapidOverride);

private:
    Communicator *m_communicator;

    int m_targetFeed    = 100;
    int m_targetSpindle = 100;
    int m_targetRapid   = 100;

    bool m_feedOverridden    = false;
    bool m_rapidOverridden   = false;
    bool m_spindleOverridden = false;

    // Sends one realtime command to move current value one step toward target.
    void sendStepCommand(int current, int target,
                         char fullRate,
                         char decrease10, char decrease1,
                         char increase10, char increase1);

    void sendRapidCommand(int current, int target);
};
