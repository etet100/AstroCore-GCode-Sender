// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "core/state_behavior/behaviors.h"

IdleBehavior::IdleBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void IdleBehavior::onMachineStateChanged(MachineState state)
{
    // Handle device state changes
    if (state == MachineState::Run) {
        // Machine started running - transition to running behavior
        // emit transition(this, new RunningBehavior(this));
    } else if (state == MachineState::Alarm) {
        // Machine entered alarm state
        emit transition(this, new AlarmBehavior());
    }
}

StateBehavior::Result IdleBehavior::doOnExit(StateBehavior *next)
{
    qDebug() << "[Behavior][Idle] Exit";

    return Result::Ok;
}

StateBehavior::Result IdleBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    assert(m_communicator != nullptr && !m_communicator.isNull());

    qDebug() << "[Behavior][Idle] Command Response:" << command << response;

    if (command == "$G") {
        m_communicator->processGCodeParserState(commandAttributes, response);

        return Result::Ok;
    }

    if (command == "$$") {
        if (!cmdStatus.ok) {
            qDebug() << "[Behavior][Idle] Error receiving device configuration.";

            return Result::Ok;
        }

        qDebug() << "[Behavior][Idle] Processing device configuration.";
        m_communicator->processDeviceConfiguration(fullResponse);

        return Result::Ok;
    }

    return Result::Unhandled;
}

bool IdleBehavior::doAction(const Action &action)
{
    if (handleMachineConfigurationActions(action)) {
        return true;
    }

    switch (action.type()) {
        case Action::Type::Home:
            qDebug() << "[Behavior][Idle] Action: Home";
            emit transition(this, new HomingBehavior());

            return true;

        case Action::Type::Run:
            {
                RunAction runAction = static_cast<const RunAction&>(action);
                qDebug() << "[Behavior][Idle] Action: Run";
                emit transition(this, new RunningBehavior(runAction.program()));
            }
            return true;

        case Action::Type::Jog:
            {
                JoggingAction joggingAction = static_cast<const JoggingAction&>(action);
                qDebug() << "[Behavior][Idle] Action: Jog" << joggingAction.vector() << "dist" << joggingAction.distance();
                emit transition(
                    this,
                    new JoggingBehavior(
                        joggingAction.vector(),
                        joggingAction.distance(),
                        joggingAction.continuous(),
                        joggingAction.feedRate(),
                        joggingAction.feedRateZ()
                    )
                );
            }
            return true;

        case Action::Type::GoTo:
            {
                GoToAction goToAction = static_cast<const GoToAction&>(action);
                qDebug() << "[Behavior][Idle] Action: GoTo" << goToAction.target() << "feed" << goToAction.feedRate();
                emit transition(this, new GoToBehavior(goToAction.target(), goToAction.feedRate()), TransitionKind::Suspend);
            }
            return true;

        case Action::Type::Probe:
            {
                // Use default parameters or extract from ProbeAction if provided
                ProbingBehavior::ProbeParameters params;

                // Try to cast to ProbeAction to get custom parameters
                const ProbeAction* probeAction = dynamic_cast<const ProbeAction*>(&action);
                if (probeAction) {
                    // Convert ProbeAction::ProbeParameters to ProbingBehavior::ProbeParameters
                    auto actionParams = probeAction->params();
                    params.fastFeedRate = actionParams.fastFeedRate;
                    params.slowFeedRate = actionParams.slowFeedRate;
                    params.maxDistance = actionParams.maxDistance;
                    params.retractDistance = actionParams.retractDistance;
                    params.safeDistance = actionParams.safeDistance;
                    params.doubleProbe = actionParams.doubleProbe;
                    params.setZeroAtProbe = actionParams.setZeroAtProbe;
                    params.useAbsolute = actionParams.useAbsolute;
                }

                qDebug() << "[Behavior][Idle] Action: Probe double=" << params.doubleProbe;
                emit transition(this, new ProbingBehavior(params));
            }
            return true;

        case Action::Type::ScanTable:
            {
                const ScanTableAction* scanTableAction = dynamic_cast<const ScanTableAction*>(&action);
                if (scanTableAction) {
                    qDebug() << "[Behavior][Idle] Action: ScanTable";
                    emit transition(this, new ScanTableBehavior(scanTableAction->heightmap()));
                } else {
                    qWarning() << "[Behavior][Idle] ScanTable action cast failed — ScanTableAction expected";
                }
            }
            return true;

        case Action::Type::ZeroZ:
            qDebug() << "[Behavior][Idle] Action: ZeroZ";
            zeroZ();

            return true;

        case Action::Type::ZeroXY:
            qDebug() << "[Behavior][Idle] Action: ZeroXY";
            zeroXY();

            return true;

        case Action::Type::Disconnect:
            qDebug() << "[Behavior][Idle] Action: Disconnect";
            emit transition(this, new DisconnectionBehavior());

            return true;

        case Action::Type::CheckMode:
            qDebug() << "[Behavior][Idle] Action: CheckMode";
            emit transition(this, new CheckModeBehavior());

            return true;
    }

    return false;
}

void IdleBehavior::zeroZ()
{
    m_communicator->sendCommand(CommandSource::System, "G10 L20 P0 Z0", TABLE_INDEX_UI);
}

void IdleBehavior::zeroXY()
{
    m_communicator->sendCommand(CommandSource::System, "G10 L20 P0 X0 Y0", TABLE_INDEX_UI);
}

StateBehavior::Result IdleBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Idle] Entry";

    communicator->startQueryingMachineState();

    return Result::Ok;
}
