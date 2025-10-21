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
        QString name() override { return "Go to..."; }
        // bool isJoggingAllowed() override { return false; }
        // bool isHomingAllowed() override { return false; }
        void onDeviceStateChanged(DeviceState state) override;
        bool onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse) override;
        Result onEntry(Communicator *communicator, StateBehavior *previous = nullptr) override;

    private:
        QPointF m_target;
        int m_feedRate;
};

#endif // GOTOBEHAVIOR_H


#endif /* F7361A36_9D66_4126_9302_5C7751E1EAC2 */
