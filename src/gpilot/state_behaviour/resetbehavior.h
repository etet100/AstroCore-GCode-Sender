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
        bool isJoggingAllowed() override { return true; }
        bool isHomingAllowed() override { return true; }
        void onDeviceState(DeviceState state) override;
        bool onRawResponse(QString response) override;
        bool onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse) override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
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
