// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef RESETBEHAVIOR_H
#define RESETBEHAVIOR_H

#include "statebehavior.h"

class ResetBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit ResetBehavior(QObject *parent = nullptr);
        QString name() override { return "Reset"; }
        void onMachineState(MachineState state) override;
        Result onRawResponse(QString response) override;
        Result onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
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
