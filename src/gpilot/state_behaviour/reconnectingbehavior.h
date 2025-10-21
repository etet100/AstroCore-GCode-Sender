// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef RECONNECTINGBEHAVIOR_H
#define RECONNECTINGBEHAVIOR_H

#include "statebehavior.h"
#include "io/connection/connection.h"

class ReconnectingBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit ReconnectingBehavior(Connection *newConnection);
        QString name() override { return "Reconnecting"; }
        // bool isJoggingAllowed() override { return false; }
        // bool isHomingAllowed() override { return false; }
        void onConnectionStateChanged(ConnectionState state) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;

    private:
        enum Stage {
            None,
            Closing,
            Completed
        };
        Stage m_stage = None;
        Connection *m_newConnection;
};

#endif // RECONNECTINGBEHAVIOR_H

