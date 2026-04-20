// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef DISCONNECTINGBEHAVIOR_H
#define DISCONNECTINGBEHAVIOR_H

#include "abstractstatebehavior.h"

// Represents the disconnecting state — the machine connection is being
// intentionally closed. The only available action is Connect, which
// transitions back to InitializationBehavior.
class DisconnectingBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        explicit DisconnectingBehavior(QObject *parent = nullptr);
        QString description() override { return "Disconnected"; }
        Type type() const override { return Type::Disconnecting; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Connect };
        }

        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;

    protected:
        QString name() const override { return "Disconnecting"; }
        bool doAction(const Action &action) override;

    private slots:
        void onConnectionStateChanged(ConnectionState state);

    private:
        int m_disconnectionTimeoutId = 0;
};

#endif // DISCONNECTINGBEHAVIOR_H
