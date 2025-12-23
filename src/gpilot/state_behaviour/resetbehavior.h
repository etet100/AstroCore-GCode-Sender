// This file is a part of "G-Pilot GCode Sender" application.

#ifndef RESETBEHAVIOR_H
#define RESETBEHAVIOR_H

#include "statebehavior.h"

class ResetBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit ResetBehavior(QObject *parent = nullptr);
        QString description() override { return "Reset"; }
        void onMachineState(MachineState state) override;
        Result onRawResponse(QString response) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        Result onEntry(CommunicatorApi *communicator, StateBehavior *previous = nullptr) override;
        void onAlarm(int code) override;

    protected:
        QString name() const override { return "ResetBehavior"; }

    private:
        enum Stage {
            None,
            SentReset,
            SentSettingsAndOffsets,
            ReceivedSettings,
            ReceivedOffsets,
            Completed
        };
        Stage m_stage = None;

        bool dataIsReset(QString data);
};

#endif // RESETBEHAVIOR_H
