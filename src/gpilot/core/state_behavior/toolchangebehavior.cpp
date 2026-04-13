// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "toolchangebehavior.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

ToolChangeBehavior::ToolChangeBehavior(int toolNumber, ToolChangeSource source, QObject *parent)
    : StateBehavior{parent}
    , m_toolNumber(toolNumber)
    , m_source(source)
    , m_changeState(ToolChangeState::MovingToSafePosition)
    , m_hasProbe(false)
{
}

QString ToolChangeBehavior::description()
{
    QString baseName = QString("Tool Change - T%1").arg(m_toolNumber);

    switch (m_changeState) {
        case ToolChangeState::MovingToSafePosition:
            return baseName + " (Moving to safe position)";
        case ToolChangeState::WaitingForUserConfirmation:
            return baseName + " (Waiting for confirmation)";
        case ToolChangeState::ReturningToWorkPosition:
            return baseName + " (Returning to work)";
        case ToolChangeState::Completed:
            return baseName + " (Completed)";
        default:
            return baseName;
    }
}

StateBehavior::Result ToolChangeBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{

    qDebug() << "[Behavior][ToolChange] Tool change requested for tool:" << m_toolNumber;

    emit stateEvent("toolChangeRequested", {{"toolNumber", m_toolNumber}});

    // Save current work position
    if (m_communicator) {
        // Get current position from machine status
        // m_savedPosition = m_communicator->workPosition();
    }

    // Start tool change sequence
    moveToSafePosition();

    return Result::Ok;
}

StateBehavior::Result ToolChangeBehavior::doOnExit(StateBehavior *next)
{
    Q_UNUSED(next);
    qDebug() << "[Behavior][ToolChange] Exiting tool change state.";
    return Result::Ok;
}

void ToolChangeBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Idle) {
        // Handle state transitions based on current tool change state
        switch (m_changeState) {
            case ToolChangeState::MovingToSafePosition:
                qDebug() << "[Behavior][ToolChange] Arrived at safe position.";
                waitForUserConfirmation();
                break;
            case ToolChangeState::ReturningToWorkPosition:
                qDebug() << "[Behavior][ToolChange] Returned to work position.";
                complete();
                break;
            default:
                break;
        }
    } else if (state == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior());
    }
}

StateBehavior::Result ToolChangeBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(command);
    Q_UNUSED(commandAttributes);
    Q_UNUSED(cmdStatus);
    Q_UNUSED(response);
    Q_UNUSED(fullResponse);

    return Result::Ok;
}

bool ToolChangeBehavior::doAction(const Action &action)
{
    // User can confirm tool change to continue
    if (action.type() == Action::Type::Resume || action.type() == Action::Type::CycleStart) {
        if (m_changeState == ToolChangeState::WaitingForUserConfirmation) {
            qDebug() << "[Behavior][ToolChange] User confirmed tool change.";
            returnToWorkPosition();
            return true;
        }
    }

    return StateBehavior::doAction(action);
}

void ToolChangeBehavior::moveToSafePosition()
{
    m_changeState = ToolChangeState::MovingToSafePosition;
    qDebug() << "[Behavior][ToolChange] Moving to safe tool change position.";

    if (m_communicator) {
        // Move to safe Z height first (G53 G0 Z-10 for example)
        // This should be configurable in machine settings
        m_communicator->sendCommand(CommandSource::System, "G53 G0 Z0", TABLE_INDEX_UI);

        // Then move to tool change position (e.g., X0 Y0)
        // m_communicator->sendCommand(CommandSource::System, "G53 G0 X0 Y0", TABLE_INDEX_UI);
    }
}

void ToolChangeBehavior::waitForUserConfirmation()
{
    m_changeState = ToolChangeState::WaitingForUserConfirmation;
    qDebug() << "[Behavior][ToolChange] Waiting for user to change tool and confirm.";

    // Emit signal or show dialog for user confirmation
    // User should press "Resume" or "Cycle Start" to continue
}

void ToolChangeBehavior::returnToWorkPosition()
{
    m_changeState = ToolChangeState::ReturningToWorkPosition;
    qDebug() << "[Behavior][ToolChange] Returning to saved work position.";

    if (m_communicator) {
        // Optionally probe tool length if probe is available
        if (m_hasProbe) {
            // Send probe command
            // m_communicator->sendCommand(CommandSource::System, "G38.2 Z-50 F100", TABLE_INDEX_UI);
        }

        // Return to saved position
        // QString cmd = QString("G0 X%1 Y%2").arg(m_savedPosition.x()).arg(m_savedPosition.y());
        // m_communicator->sendCommand(CommandSource::System, cmd, TABLE_INDEX_UI);
    }
}

void ToolChangeBehavior::complete()
{
    m_changeState = ToolChangeState::Completed;
    qDebug() << "[Behavior][ToolChange] Tool change completed for tool:" << m_toolNumber;

    emit stateEvent("toolChangeCompleted", {{"toolNumber", m_toolNumber}});

    // Return to previous state (usually RunningBehavior) or IdleBehavior
    if (m_previousType == Type::Running) {
        emit resumePrevious();
    } else {
        emit transition(this, new IdleBehavior());
    }
}
