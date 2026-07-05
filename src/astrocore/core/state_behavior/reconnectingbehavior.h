// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef RECONNECTINGBEHAVIOR_H
#define RECONNECTINGBEHAVIOR_H

#include "abstractstatebehavior.h"
#include "io/connection/abstractconnection.h"

class ReconnectingBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        explicit ReconnectingBehavior(AbstractConnection *newConnection);
        QString description() override { return "Reconnecting"; }
        Type type() const override { return Type::Reconnecting; }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        // Disconnected events during reconnection are expected — the polling
        // timer in doOnEntry drives the lifecycle. Don't fall through to the
        // base default that would transition to DisconnectingBehavior.
        void onConnectionStateChanged(ConnectionState state) override { Q_UNUSED(state); }

    protected:
        QString name() const override { return "Reconnecting"; }

    private:
        enum Stage {
            None,
            Closing,
            Completed
        };
        Stage m_stage = None;
        AbstractConnection *m_newConnection;
};

#endif // RECONNECTINGBEHAVIOR_H

