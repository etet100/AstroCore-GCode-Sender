// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/core.h"
#include "core/communicator/communicator.h"
#include "pausebehavior.h"
#include "runningbehavior.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"

PauseBehavior::PauseBehavior(PauseSource source, QObject *parent)
    : AbstractStateBehavior{parent}
    , m_source(source)
{
}

bool PauseBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::Resume:
            resume();
            return true;

        case Action::Type::Abort:
            abort();
            return true;
    }

    return AbstractStateBehavior::doAction(action);
}

QString PauseBehavior::description()
{
    QString baseName = "Paused";

    // Add pause type information
    switch (m_source) {
        case PauseSource::Program:
            return baseName + " (Program)";
        case PauseSource::Jogging:
            return baseName + " (Jogging)";
        case PauseSource::UserRequest:
            return baseName + " (User Requested)";
        case PauseSource::External:
            return baseName + " (External Hold)";
        default:
            return baseName;
    }
}

AbstractStateBehavior::Result PauseBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Pause] Entry";

    // Perform different actions based on pause source
    switch (m_source) {
        case PauseSource::Program:
            // For example, stop spindle during program pause
            // m_communicator->sendCommand(CommandSource::System, "M5", TABLE_INDEX_UI);
            break;
        case PauseSource::Jogging:
            // During jogging pause, may want to take other actions
            break;
        default:
            break;
    }

    Core::instance().timer().pauseExecution();
    communicator->startQueryingMachineState();

    return AbstractStateBehavior::Result::Ok;
}

AbstractStateBehavior::Result PauseBehavior::doOnExit(AbstractStateBehavior *next)
{
    return Result::Ok;
}

void PauseBehavior::onMachineStateChanged(MachineState state)
{
    if (state == MachineState::Run) {
        // Based on pause source, return to appropriate state
        switch (m_source) {
            case PauseSource::Program:
                // For program pause, return to Running state
                // emit transition(this, new RunningBehavior(this));
                break;
            case PauseSource::Jogging:
                // For jogging pause, return to Jogging state
                // emit transition(this, new JoggingBehavior(this));
                break;
            case PauseSource::UserRequest:
            case PauseSource::External:
            default:
                // For other sources, return to previous state or Running
                if (m_previousType.has_value()) {
                    emit resumePrevious();
                } else {
                    // emit transition(this, new RunningBehavior(this));
                }
                break;
        }
    } else if (state == MachineState::Alarm) {
        // Transition to Alarm state
        emit transition(this, new AlarmBehavior());
    } else if (state == MachineState::Idle) {
        emit transition(this, new IdleBehavior());
    }
}

AbstractStateBehavior::Result PauseBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    return Result::Ok;
}

void PauseBehavior::resume()
{
    qDebug() << "[Behavior][Pause] Resuming";

    m_action = PauseAction::Resume;
    setExitValue("action", "resume");

    emit resumePrevious();
}

void PauseBehavior::abort()
{
    qDebug() << "[Behavior][Pause] Aborting";

    m_action = PauseAction::Abort;
    setExitValue("action", "abort");

    emit resumePrevious();
}
