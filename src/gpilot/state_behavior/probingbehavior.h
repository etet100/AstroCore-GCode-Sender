#ifndef PROBINGBEHAVIOR_H
#define PROBINGBEHAVIOR_H

#include "statebehavior.h"

class ProbingBehavior : public StateBehavior
{
    public:
        explicit ProbingBehavior(QObject* parent = nullptr);
        QString description() override { return "Probing"; }
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        Result onExit(StateBehavior *next = nullptr) override;
        void onAlarm(int code) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        void onMachineStateChanged(MachineState state) override;

    protected:
        QString name() const override { return "ProbingBehavior"; }

    private:
        bool m_alarm = false;
        int m_alarmCode;
};

#endif // PROBINGBEHAVIOR_H
