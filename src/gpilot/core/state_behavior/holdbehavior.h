// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef HOLDBEHAVIOR_H
#define HOLDBEHAVIOR_H

#include "abstractstatebehavior.h"

class HoldBehavior : public AbstractStateBehavior
{
    Q_OBJECT

    public:
        enum class HoldSource {
            UserRequest,    // Manual feed hold by user (!)
            Door,           // Safety door triggered
            Emergency       // Emergency stop
        };

        explicit HoldBehavior(HoldSource source = HoldSource::UserRequest, QObject *parent = nullptr);
        QString description() override;
        Type type() const override { return Type::Hold; }
        QSet<Action::Type> availableActions() const override {
            return { Action::Resume, Action::CycleStart };
        }
        Result doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx) override;
        Result doOnExit(AbstractStateBehavior *next) override;
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;

    protected:
        QString name() const override { return "Hold"; }
        bool doAction(const Action &action) override;

    private:
        HoldSource m_source;
        void resume();
};

#endif // HOLDBEHAVIOR_H
