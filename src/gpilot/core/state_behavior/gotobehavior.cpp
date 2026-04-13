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

void GoToBehavior::doOnMachineState(MachineState state)
{

    if (m_stage == CommandSent && (state == MachineState::Jog || state == MachineState::Run)) {
        m_stage = WaitingForMovementEnd;
        // // Movement completed, return to previous state or idle
        // if (m_previous) {
        //     emit resumePrevious();
        // } else {
        //     emit transition(this, new IdleBehavior(this));
        // }
    } else if (state == MachineState::Idle && m_stage == WaitingForMovementEnd) {
        m_stage = Completed;
        m_communicator->stopQueryingMachineState();

        emit resumePrevious();
    }
}

void GoToBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][GoTo] Alarm during go to:" << code;

    emit transition(this, new AlarmBehavior(code));
}

bool GoToBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Abort:
            stopJogging();

            return true;
    }

    return false;
}

void GoToBehavior::stopJogging()
{
    qDebug() << "[Behavior][GoTo] Stopping";

    qDebug() << "[Behavior][GoTo] Send JOG CANCEL and clear queue";
    m_communicator->clearQueue();
    m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);

    qDebug() << "[Behavior][GoTo] Now wait for idle state";
    setTimeout(500, [this]() {
        qWarning() << "[Behavior][GoTo] No response after stop command, transitioning to Error";

        emit transition(this, new ErrorBehavior("Failed to stop GoTo movement"));
    });
}

StateBehavior::Result GoToBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);
    Q_UNUSED(cmdStatus);
    Q_UNUSED(fullResponse);

    if (!cmdStatus.ok) {
        qDebug() << "[Behavior][GoTo] Command Error:" << cmdStatus.errorCode;
        log("Go to command failed with error " + QString::number(cmdStatus.errorCode), {"Behavior", "GoTo"});

        emit resumePrevious();

        return Result::Ok;
    }

    qDebug() << "[Behavior][GoTo] Command Response:" << command << response;

    return Result::Ok;
}

StateBehavior::Result GoToBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][GoTo] Entry with target:" << m_target << "feed rate:" << m_feedRate;

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
