#include "overrides.h"
#include "communicator.h"
#include "core/globals.h"
#include <cmath>

Overrides::Overrides(Communicator *communicator)
    : QObject(communicator)
    , m_communicator(communicator)
{
    connect(communicator, &Communicator::overridesReceived,
            this, &Overrides::onOverridesReceived);
}

void Overrides::setTargets(bool feedOverridden, int feed,
                           bool rapidOverridden, int rapid,
                           bool spindleOverridden, int spindle)
{
    m_feedOverridden    = feedOverridden;
    m_rapidOverridden   = rapidOverridden;
    m_spindleOverridden = spindleOverridden;

    m_targetFeed    = feed;
    m_targetSpindle = spindle;
    m_targetRapid   = rapid;
}

void Overrides::onOverridesReceived(int feedOverride, int spindleOverride, int rapidOverride)
{
    sendStepCommand(feedOverride, m_targetFeed,
                    GRBL_LIVE_FEED_FULL_RATE,
                    GRBL_LIVE_FEED_DECREASE_10, GRBL_LIVE_FEED_DECREASE_1,
                    GRBL_LIVE_FEED_INCREASE_10, GRBL_LIVE_FEED_INCREASE_1);

    sendStepCommand(spindleOverride, m_targetSpindle,
                    GRBL_LIVE_SPINDLE_FULL_SPEED,
                    GRBL_LIVE_SPINDLE_DECREASE_10, GRBL_LIVE_SPINDLE_DECREASE_1,
                    GRBL_LIVE_SPINDLE_INCREASE_10, GRBL_LIVE_SPINDLE_INCREASE_1);

    sendRapidCommand(rapidOverride, m_targetRapid);

    emit currentValuesChanged(feedOverride, spindleOverride, rapidOverride);
}

// Sends one GRBL realtime command to move 'current' one step toward 'target'.
void Overrides::sendStepCommand(int current, int target,
                                char fullRate,
                                char decrease10, char decrease1,
                                char increase10, char increase1)
{
    if (current == target) {
        return;
    }

    if (target == 100) {
        m_communicator->sendRealtimeCommand(fullRate);
        return;
    }

    int diff = std::abs(target - current);
    char cmd = (target < current)
        ? (diff >= 10 ? decrease10 : decrease1)
        : (diff >= 10 ? increase10 : increase1);

    m_communicator->sendRealtimeCommand(cmd);
}

// Sends one GRBL realtime command for rapid override (only 3 discrete levels).
void Overrides::sendRapidCommand(int current, int target)
{
    if (current == target) {
        return;
    }

    switch (target) {
        case 25:
            m_communicator->sendRealtimeCommand(GRBL_LIVE_RAPID_QUARTER_RATE);
            break;
        case 50:
            m_communicator->sendRealtimeCommand(GRBL_LIVE_RAPID_HALF_RATE);
            break;
        case 100:
            m_communicator->sendRealtimeCommand(GRBL_LIVE_RAPID_FULL_RATE);
            break;
    }
}
