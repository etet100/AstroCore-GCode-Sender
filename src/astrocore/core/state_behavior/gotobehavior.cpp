// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "gotobehavior.h"
#include "behaviors.h"
#include "core/communicator/communicator.h"

using namespace std::chrono_literals;

GoToBehavior::GoToBehavior(QPointF target, int feedRate,
                           bool delegateAlarmToParent, QObject *parent)
    : AbstractStateBehavior{parent}
    , m_target(target)
    , m_feedRate(feedRate)
    , m_delegateAlarmToParent(delegateAlarmToParent)
{}

void GoToBehavior::emitAlarmExit()
{
    if (m_delegateAlarmToParent) {
        setExitValue({
            {"success", false},
            {"alarmOccurred", true},
            {"alarmCode", m_alarmCode},
        });
        emit resumePrevious();
    } else {
        emit transition(this, new AlarmBehavior(m_alarmCode));
    }
}

void GoToBehavior::emitFailureExit(const QString &reason)
{
    if (m_delegateAlarmToParent) {
        setExitValue({
            {"success", false},
            {"failureReason", reason},
        });
        emit resumePrevious();
    } else {
        emit transition(this, new ErrorBehavior("GoTo: " + reason));
    }
}

AbstractStateBehavior::Result GoToBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][GoTo] Entry with target:" << m_target << "feed rate:" << m_feedRate;

    m_goToTask = runGoToSequence();

    return Result::Ok;
}

AbstractStateBehavior::Result GoToBehavior::doOnExit(AbstractStateBehavior *next)
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
        QString reason;
        if (r) {
            reason = QString("command rejected, error %1").arg(r->status.errorCode);
            qDebug() << "[Behavior][GoTo]" << reason;
            log("Go to command failed with error " + QString::number(r->status.errorCode), {"GoTo", "Error"});
        } else {
            reason = "command timed out";
            qWarning() << "[Behavior][GoTo] Command timed out";
        }
        emitFailureExit(reason);
        co_return;
    }

    if (m_alarmOccurred) {
        emitAlarmExit();
        co_return;
    }

    // Wait up to 500ms for movement to begin (Jog or Run).
    // If the move is so short that the machine never leaves Idle, treat it as done.
    auto moving = co_await awaitMachineState(
        [](MachineState s) { return s == MachineState::Jog || s == MachineState::Run; },
        500ms
    );

    if (m_alarmOccurred) {
        emitAlarmExit();
        co_return;
    }

    if (!moving) {
        qDebug() << "[Behavior][GoTo] Short move: no Jog/Run observed within 500ms, resuming";
        setExitValue("success", true);
        emit resumePrevious();
        co_return;
    }

    // Wait for movement to finish
    auto idle = co_await awaitMachineState(
        [](MachineState s) { return s == MachineState::Idle; },
        120s
    );

    if (m_alarmOccurred) {
        emitAlarmExit();
        co_return;
    }

    if (!idle) {
        qWarning() << "[Behavior][GoTo] Timeout waiting for movement to complete";
        emitFailureExit("movement timed out");
        co_return;
    }

    qDebug() << "[Behavior][GoTo] Movement completed";
    setExitValue("success", true);
    emit resumePrevious();
}

void GoToBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][GoTo] Alarm during go to:" << code;

    m_alarmOccurred = true;
    m_alarmCode = code;

    if (m_delegateAlarmToParent) {
        // Wake any coroutine currently inside awaitMachineState so it can
        // observe m_alarmOccurred and emit the proper delegation exit
        // without waiting for the full await timeout.
        emit machineStateSignal(MachineState::Alarm);
    } else {
        emit transition(this, new AlarmBehavior(code));
    }
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
