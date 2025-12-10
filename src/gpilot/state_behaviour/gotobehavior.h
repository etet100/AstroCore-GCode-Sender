// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef F7361A36_9D66_4126_9302_5C7751E1EAC2
#define F7361A36_9D66_4126_9302_5C7751E1EAC2

#ifndef GOTOBEHAVIOR_H
#define GOTOBEHAVIOR_H

#include "statebehavior.h"

class GoToBehavior : public StateBehavior
{
    public:
        explicit GoToBehavior(QPointF target, int feedRate, QObject *parent = nullptr);
        QString description() override { return "Go to..."; }
        void onMachineState(MachineState state) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;

    protected:
        QString name() const override { return "GoToBehavior"; }

    private:
        enum Stage {
            None,
            CommandSent,
            WaitingForMovementEnd,
            Completed
        };

        QPointF m_target;
        int m_feedRate;
        Stage m_stage = None;
};

#endif // GOTOBEHAVIOR_H


#endif /* F7361A36_9D66_4126_9302_5C7751E1EAC2 */
