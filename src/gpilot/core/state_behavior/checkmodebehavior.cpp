// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "checkmodebehavior.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

CheckModeBehavior::CheckModeBehavior(QObject *parent)
    : AbstractStateBehavior{parent}
{
}

QString CheckModeBehavior::description()
{
    return "Check Mode (Dry Run)";
}

AbstractStateBehavior::Result CheckModeBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    Q_UNUSED(ctx);

    qDebug() << "[Behavior][CheckMode] Entry — sending $C, waiting for Check state";

    m_stage = Stage::Entering;
    communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);
    communicator->startQueryingMachineState();

    return Result::Ok;
}

void CheckModeBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior());

        return;
    }

    switch (m_stage) {
        case Stage::Entering:
            if (state == MachineState::Check) {
                qDebug() << "[Behavior][CheckMode] Check state reached";
                m_stage = Stage::Active;
            } else if (state == MachineState::Idle) {
                // $C did not toggle check mode — machine stayed Idle
                qWarning() << "[Behavior][CheckMode] Machine returned to Idle before reaching Check";
                emit transition(this, new IdleBehavior());
            }
            break;

        case Stage::Active:
            if (state == MachineState::Idle) {
                // External exit from check mode
                qDebug() << "[Behavior][CheckMode] Check mode exited externally";
                emit transition(this, new IdleBehavior());
            }
            break;

        case Stage::Exiting:
            if (state == MachineState::Idle) {
                qDebug() << "[Behavior][CheckMode] Check mode exited";
                emit transition(this, new IdleBehavior());
            }
            break;
    }
}

void CheckModeBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][CheckMode] Alarm during check mode:" << code;
    emit transition(this, new AlarmBehavior(code));
}

bool CheckModeBehavior::doAction(const Action &action)
{
    if (action.type() == Action::Type::Abort && m_stage == Stage::Active) {
        qDebug() << "[Behavior][CheckMode] Abort — sending $C, waiting for Idle state";
        m_stage = Stage::Exiting;
        m_communicator->sendCommand(CommandSource::System, "$C", TABLE_INDEX_UI);

        return true;
    }

    return AbstractStateBehavior::doAction(action);
}
