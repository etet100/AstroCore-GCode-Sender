// This file is a part of "G-Pilot GCode Sender" application.

#ifndef IDLEBEHAVIOR_H
#define IDLEBEHAVIOR_H

#include "statebehavior.h"

class IdleBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit IdleBehavior(QObject *parent = nullptr);
        QString description() override { return "Idle"; }
        Type type() const override { return Type::Idle; }
        QSet<Action::Type> availableActions() const override {
            return {
                Action::Run,
                Action::Home,
                Action::Jog,
                Action::GoTo,
                Action::Probe,
                Action::ZeroZ,
                Action::ZeroXY,
                Action::Disconnect,
            };
        }
        void onMachineStateChanged(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;

    protected:
        QString name() const override { return "Idle"; }
        bool doAction(const Action &action) override;

    private:
        void zeroZ();
        void zeroXY();
};

#endif // IDLEBEHAVIOR_H
