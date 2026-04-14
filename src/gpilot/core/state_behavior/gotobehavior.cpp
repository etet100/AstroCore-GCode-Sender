// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "gotobehavior.h"
#include "behaviors.h"
#include "core/communicator/communicator.h"

using namespace std::chrono_literals;

GoToBehavior::GoToBehavior(QPointF target, int feedRate, QObject *parent)
    : StateBehavior{parent}
    , m_target(target)
    , m_feedRate(feedRate)
{}

StateBehavior::Result GoToBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][GoTo] Entry with target:" << m_target << "feed rate:" << m_feedRate;

    m_goToTask = runGoToSequence();

    return Result::Ok;
}

StateBehavior::Result GoToBehavior::doOnExit(StateBehavior *next)
{
    qDebug() << "[Behavior][GoTo] Exit";

    m_goToTask.reset();
    m_communicator->stopQueryingMachineState();

    return Result::Ok;
}

QCoro::Task<void> GoToBehavior::runGoToSequence()
{
    QString cmd = QString("$J=G90 X%1 Y%2 F%3")
        .arg(m_target.x())
        .arg(m_target.y())
        .arg(m_feedRate);

    m_communicator->startQueryingMachineState();

    // Send jog command and wait for acceptance
    auto r = co_await sendAndAwait(cmd, 5s);

    if (!r || !r->status.ok) {
        if (r) {
            qDebug() << "[Behavior][GoTo] Command rejected, error" << r->status.errorCode;
            log("Go to command failed with error " + QString::number(r->status.errorCode), {"GoTo", "Error"});
        } else {
            qWarning() << "[Behavior][GoTo] Command timed out";
        }
        emit resumePrevious();
        co_return;
    }

    if (m_alarmOccurred) {
        emit transition(this, new AlarmBehavior(m_alarmCode));
        co_return;
    }

    // Wait up to 500ms for movement to begin (Jog or Run).
    // If the move is so short that the machine never leaves Idle, treat it as done.
    auto moving = co_await awaitMachineState(
        [](MachineState s) { return s == MachineState::Jog || s == MachineState::Run; },
        500ms
    );

    if (m_alarmOccurred) {
        emit transition(this, new AlarmBehavior(m_alarmCode));
        co_return;
    }

    if (!moving) {
        qDebug() << "[Behavior][GoTo] Short move: no Jog/Run observed within 500ms, resuming";
        emit resumePrevious();
        co_return;
    }

    // Wait for movement to finish
    auto idle = co_await awaitMachineState(
        [](MachineState s) { return s == MachineState::Idle; },
        120s
    );

    if (m_alarmOccurred) {
        emit transition(this, new AlarmBehavior(m_alarmCode));
        co_return;
    }

    if (!idle) {
        qWarning() << "[Behavior][GoTo] Timeout waiting for movement to complete";
        emit transition(this, new ErrorBehavior("GoTo movement timed out"));
        co_return;
    }

    qDebug() << "[Behavior][GoTo] Movement completed";
    emit resumePrevious();
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
    qDebug() << "[Behavior][GoTo] Stopping — sending JOG CANCEL";

    m_communicator->clearQueue();
    m_communicator->sendRealtimeCommand(GRBL_LIVE_JOG_CANCEL);
}
