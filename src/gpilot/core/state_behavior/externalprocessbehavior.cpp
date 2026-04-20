// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "externalprocessbehavior.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

ExternalProcessBehavior::ExternalProcessBehavior(QObject *parent)
    : AbstractStateBehavior{parent}
{}

QString ExternalProcessBehavior::description() { return "External process"; }

AbstractStateBehavior::Result ExternalProcessBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][ExternalProcess] Entry — machine is running an external process.";

    log("[ExternalProcess] Machine is executing an external process. Monitoring state.");
    m_communicator->startQueryingMachineState();

    return Result::Ok;
}

void ExternalProcessBehavior::onMachineStateChanged(MachineState state)
{
    qDebug() << "[Behavior][ExternalProcess] Machine state changed:" << static_cast<int>(state);

    switch (state) {
        case MachineState::Idle:
            log("[ExternalProcess] External process finished. Machine is idle.");
            emit transition(this, new IdleBehavior());
            break;

        case MachineState::Alarm:
            log("[ExternalProcess] Machine entered alarm state.");
            emit transition(this, new AlarmBehavior(m_communicator->lastAlarmCode()));
            break;

        default:
            break;
    }
}

void ExternalProcessBehavior::onAlarm(int code)
{
    emit transition(this, new AlarmBehavior(code));
}

bool ExternalProcessBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Disconnect:
            emit transition(this, new DisconnectingBehavior());

            return true;
    }

    return false;
}
