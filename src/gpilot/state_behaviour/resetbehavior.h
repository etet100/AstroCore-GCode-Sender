// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef B7040668_E41E_4691_831C_8895FBBBC2FD
#define B7040668_E41E_4691_831C_8895FBBBC2FD

#ifndef RESETBEHAVIOR_H
#define RESETBEHAVIOR_H

#include "statebehavior.h"

class ResetBehavior : public StateBehavior
{
    public:
        explicit ResetBehavior(QObject *parent = nullptr);
        QString name() override { return "Reset"; }
        bool isJoggingAllowed() override { return true; }
        bool isHomingAllowed() override { return true; }
        void onDeviceStateChanged(DeviceState state) override;
        void onCommandResponse(QString command, CommandAttributes commandAttributes, QStringList response) override;
        void onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
    private:
        bool dataIsReset(QString data);
};

#endif // RESETBEHAVIOR_H


#endif /* B7040668_E41E_4691_831C_8895FBBBC2FD */
