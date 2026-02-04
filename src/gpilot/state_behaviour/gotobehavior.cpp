// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "gotobehavior.h"
#include "behaviors.h"
#include "core/communicator/communicator.h"

GoToBehavior::GoToBehavior(QPointF target, int feedRate, QObject *parent)
    : StateBehavior{parent}
    , m_target(target)
    , m_feedRate(feedRate)
{}

void GoToBehavior::onMachineState(MachineState state)
{
    if (m_stage == CommandSent && (state == MachineState::Jog || state == MachineState::Run)) {
        m_stage = WaitingForMovementEnd;
        // // Movement completed, return to previous state or idle
        // if (m_previous) {
        //     emit transition(this, m_previous);
        // } else {
        //     emit transition(this, new IdleBehavior(this));
        // }
    } else if (state == MachineState::Idle && m_stage == WaitingForMovementEnd) {
        m_stage = Completed;
        m_communicator->stopQueryingMachineState();

        transitionToPreviousState();
    }
}

void GoToBehavior::onAlarm(int code)
{
    qDebug() << "[GoToBehavior] Alarm during go to:" << code;

    emit transition(this, new AlarmBehavior(code));
}

StateBehavior::Result GoToBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);
    Q_UNUSED(cmdStatus);
    Q_UNUSED(fullResponse);

    if (!cmdStatus.ok) {
        qDebug() << "[GoToBehavior] Command Error:" << cmdStatus.errorCode;
        log("Go to command failed with error " + QString::number(cmdStatus.errorCode), {"GoToBehavior"});

        transitionToPreviousState();

        return Result::Ok;
    }

    qDebug() << "[GoToBehavior] Command Response:" << command << response;

    return Result::Ok;
}

StateBehavior::Result GoToBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[GoToBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    // QString cmd = QString("G1 X%1 Y%2 F%3")
    QString cmd = QString("$J=G90 X%1 Y%2 F%3")
        .arg(m_target.x())
        .arg(m_target.y())
        .arg(m_feedRate);

    communicator->sendCommand(CommandSource::System, cmd, TABLE_INDEX_UI);
    communicator->queryMachineState();
    communicator->startQueryingMachineState();

    m_stage = CommandSent;

    return Result::Ok;
}
