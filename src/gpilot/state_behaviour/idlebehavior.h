// This file is a part of "G-Pilot (formerly Candle)" application.

#ifndef IDLEBEHAVIOR_H
#define IDLEBEHAVIOR_H

#include "statebehavior.h"

class IdleBehavior : public StateBehavior
{
    Q_OBJECT

    public:
        explicit IdleBehavior(QObject *parent = nullptr);
        QString name() override { return "Idle"; }
        void onDeviceStateChanged(DeviceState state) override;
        bool onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;
};

#endif // IDLEBEHAVIOR_H
