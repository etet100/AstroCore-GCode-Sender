// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CHECKMODEBEHAVIOR_H
#define CHECKMODEBEHAVIOR_H

#include "abstractstatebehavior.h"

class CheckModeBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        explicit CheckModeBehavior(QObject *parent = nullptr);
        QString description() override;
        Type type() const override { return Type::CheckMode; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Abort };
        }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        void onMachineStateChanged(MachineState state) override;
        void onAlarm(int code) override;

    protected:
        QString name() const override { return "CheckMode"; }
        bool doAction(const Action &action) override;

    private:
        enum class Stage {
            Entering,   // $C sent, waiting for Check state
            Active,     // in Check state
            Exiting,    // $C sent to leave, waiting for Idle state
        };

        Stage m_stage = Stage::Entering;
};

#endif // CHECKMODEBEHAVIOR_H
