// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef CONNECTINGBEHAVIOR_H
#define CONNECTINGBEHAVIOR_H

#include "abstractstatebehavior.h"

class ConnectingBehavior : public AbstractStateBehavior
{
    public:
        explicit ConnectingBehavior(QObject *parent = nullptr);
        QString description() override;
        Type type() const override { return Type::Connecting; }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;
        void onConnectionStateChanged(ConnectionState state) override;

    protected:
        QString name() const override { return "Connecting"; }

    private:
        // bool dataIsReset(QString data);
};

#endif // CONNECTINGBEHAVIOR_H
