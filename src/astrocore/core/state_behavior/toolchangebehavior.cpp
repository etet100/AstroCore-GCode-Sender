// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "toolchangebehavior.h"
#include "runningbehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"
#include "userpromptbehavior.h"
#include "core/communicator/communicator.h"

ToolChangeBehavior::ToolChangeBehavior(int toolNumber, ToolChangeSource source, QObject *parent)
    : AbstractStateBehavior{parent}
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

AbstractStateBehavior::Result ToolChangeBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    Q_UNUSED(communicator);

    // Resumed from the confirmation prompt (see waitForUserConfirmation()).
    if (ctx.previousType == Type::UserPrompt) {
        const QString choiceId = ctx.data.value("choiceId").toString();
        qDebug() << "[Behavior][ToolChange] Resumed from prompt with choice:" << choiceId;

        if (choiceId == "confirm") {
            returnToWorkPosition();
        } else {
            log("Tool change aborted by user", {"ToolChange"});
            emit transition(this, new IdleBehavior());
        }

        return Result::Ok;
    }

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

AbstractStateBehavior::Result ToolChangeBehavior::doOnExit(AbstractStateBehavior *next)
{
    Q_UNUSED(next);
    qDebug() << "[Behavior][ToolChange] Exiting tool change state.";
    return Result::Ok;
}

void ToolChangeBehavior::onMachineStateChanged(MachineState state)
{
    // Idle transitions are driven by waitForIdle() (poll-based), not here —
    // see the comment on waitForIdle() for why the change signal is unreliable.
    if (state == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior());
    }
}

AbstractStateBehavior::Result ToolChangeBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(command);
    Q_UNUSED(commandAttributes);
    Q_UNUSED(cmdStatus);
    Q_UNUSED(response);
    Q_UNUSED(fullResponse);

    return Result::Ok;
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

    waitForIdle([this]() {
        qDebug() << "[Behavior][ToolChange] Arrived at safe position.";
        waitForUserConfirmation();
    });
}

void ToolChangeBehavior::waitForUserConfirmation()
{
    m_changeState = ToolChangeState::WaitingForUserConfirmation;
    qDebug() << "[Behavior][ToolChange] Waiting for user to change tool and confirm.";

    PromptSpec spec;
    spec.promptId = "toolchange.confirm";
    spec.title = QString("Tool change — T%1").arg(m_toolNumber);
    spec.message = QString("Insert tool T%1 and confirm to continue.").arg(m_toolNumber);
    spec.context = {{"toolNumber", m_toolNumber}};
    spec.choices = {
        {"confirm", "Confirm", /*destructive=*/false, /*isDefault=*/true},
        {"abort", "Abort", /*destructive=*/true, /*isDefault=*/false},
    };

    emit transition(this, new UserPromptBehavior(spec), TransitionKind::Suspend);
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

    waitForIdle([this]() {
        qDebug() << "[Behavior][ToolChange] Returned to work position.";
        complete();
    });
}

void ToolChangeBehavior::waitForIdle(std::function<void()> onIdle)
{
    if (!m_communicator) {
        return;
    }

    waitForStateResponse([this, onIdle = std::move(onIdle)](MachineState) {
        // Premature Idle: the move command is not fully acknowledged yet, so
        // the machine has not actually started moving. Keep waiting.
        if (!m_communicator->isCommandBufferEmpty() || !m_communicator->isQueueEmpty()) {
            waitForIdle(onIdle);

            return;
        }

        onIdle();
    }, MachineState::Idle);
}

void ToolChangeBehavior::complete()
{
    m_changeState = ToolChangeState::Completed;
    qDebug() << "[Behavior][ToolChange] Tool change completed for tool:" << m_toolNumber;

    emit stateEvent("toolChangeCompleted", {{"toolNumber", m_toolNumber}});

    // Return to the parent. For program-driven tool changes the parent is
    // always RunningBehavior on the suspended stack — m_previousType can't
    // be relied on here because it's overwritten by intermediate states
    // (e.g. the UserPromptBehavior that just resumed us).
    if (m_source == ToolChangeSource::Program) {
        emit resumePrevious();
    } else {
        emit transition(this, new IdleBehavior());
    }
}
