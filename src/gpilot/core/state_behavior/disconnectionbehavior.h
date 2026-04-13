// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef DISCONNECTIONBEHAVIOR_H
#define DISCONNECTIONBEHAVIOR_H

#include "statebehavior.h"

// Represents the disconnected state — the machine connection has been
// intentionally closed. The only available action is Connect, which
// transitions back to InitializationBehavior.
class DisconnectionBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit DisconnectionBehavior(QObject *parent = nullptr);
        QString description() override { return "Disconnected"; }
        Type type() const override { return Type::Disconnection; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Connect };
        }

        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(StateBehavior *next) override;

    protected:
        QString name() const override { return "Disconnection"; }
        bool doAction(const Action &action) override;

    private slots:
        void onConnectionStateChanged(ConnectionState state);

    private:
        int m_disconnectionTimeoutId = 0;
};

#endif // DISCONNECTIONBEHAVIOR_H
