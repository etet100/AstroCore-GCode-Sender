// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "holdbehavior.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
#include "core/communicator/communicator.h"

HoldBehavior::HoldBehavior(HoldSource source, QObject *parent)
    : StateBehavior{parent}
    , m_source(source)
{
}

QString HoldBehavior::description()
{
    QString baseName = "Feed Hold";

    switch (m_source) {
        case HoldSource::UserRequest:
            return baseName + " (User)";
        case HoldSource::Door:
            return baseName + " (Safety Door)";
        case HoldSource::Emergency:
            return baseName + " (Emergency)";
        default:
            return baseName;
    }
}

StateBehavior::Result HoldBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    qDebug() << "[HoldBehavior] Entering feed hold state.";

    // If user requested hold, send feed hold command
    if (m_source == HoldSource::UserRequest && m_communicator) {
        m_communicator->sendRealtimeCommand(GRBL_LIVE_FEED_HOLD);
    }

    return Result::Ok;
}

StateBehavior::Result HoldBehavior::onExit(StateBehavior *next)
{
    Q_UNUSED(next);
    qDebug() << "[HoldBehavior] Exiting feed hold state.";
    return StateBehavior::onExit(next);
}

void HoldBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Run) {
        qDebug() << "[HoldBehavior] Machine resumed. Checking previous state.";

        // Return to previous state if it was running
        if (m_previous && m_previous->name() == "RunningBehavior") {
            // Don't create new behavior, just signal that hold is released
            emit transition(this, m_previous);
        } else {
            // Otherwise go to idle
            emit transition(this, new IdleBehavior());
        }
    } else if (state == MachineState::Idle) {
        qDebug() << "[HoldBehavior] Machine became idle.";
        emit transition(this, new IdleBehavior());
    }
}

StateBehavior::Result HoldBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(command);
    Q_UNUSED(commandAttributes);
    Q_UNUSED(cmdStatus);
    Q_UNUSED(response);
    Q_UNUSED(fullResponse);

    // During hold, we don't process command responses
    return Result::Ok;
}

bool HoldBehavior::doAction(const Action &action)
{
    if (action.type() == Action::Type::Resume || action.type() == Action::Type::CycleStart) {
        resume();
        return true;
    }

    return StateBehavior::doAction(action);
}

void HoldBehavior::resume()
{
    qDebug() << "[HoldBehavior] Resuming from feed hold.";

    if (m_communicator) {
        // Send cycle start command to resume
        m_communicator->sendRealtimeCommand(GRBL_LIVE_CYCLE_START);
    }
}
