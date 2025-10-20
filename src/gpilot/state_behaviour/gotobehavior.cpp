// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2024 BTS

#include "gotobehavior.h"
#include "behaviors.h"
#include "core/communicator/communicator.h"

GoToBehavior::GoToBehavior(QPointF target, int feedRate, QObject *parent)
    : StateBehavior{parent}
    , m_target(target)
    , m_feedRate(feedRate)
{}

void GoToBehavior::onDeviceStateChanged(DeviceState state)
{
    if (state == DeviceState::Idle) {
        // Movement completed, return to previous state or idle
        if (m_previous) {
            emit transition(this, m_previous);
        } else {
            emit transition(this, new IdleBehavior(this));
        }
    } else if (state == DeviceState::Alarm) {
        // Movement interrupted by alarm
        emit transition(this, new AlarmBehavior());
    }
}

bool GoToBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, QString response, QStringList fullResponse)
{
    qDebug() << "[GoToBehavior] Command Response:" << command << response;
}

void GoToBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[GoToBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    QString cmd = QString("G0 X%1 Y%2 F%3")
        .arg(m_target.x())
        .arg(m_target.y())
        .arg(m_feedRate);

    communicator->sendCommand(CommandSource::System, cmd, TABLE_INDEX_UI);
}
