// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "state_behaviour/behaviors.h"

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

StateBehavior::Result IdleBehavior::onExit(StateBehavior *next)
{
    qDebug() << "[IdleBehavior] Exit";

    return StateBehavior::onExit(next);
}

StateBehavior::Result IdleBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    assert(m_communicator != nullptr && !m_communicator.isNull());

    qDebug() << "[IdleBehavior] Command Response:" << command << response;

    if (command == "$G") {
        m_communicator->processGCodeParserState(commandAttributes, response);

        return Result::Ok;
    }

    if (command == "$$") {
        if (!cmdStatus.ok) {
            qDebug() << "[IdleBehavior] Error receiving device configuration.";

            return Result::Ok;
        }

        qDebug() << "[IdleBehavior] Processing device configuration.";
        m_communicator->processDeviceConfiguration(fullResponse);

        return Result::Ok;
    }

    return Result::Unhandled;;
}

bool IdleBehavior::doAction(const Action &action)
{
    switch (action.type()) {
        case Action::Type::QueryMachineConfiguration:
            m_communicator->queryMachineConfiguration();
            return true;

        case Action::Type::SaveMachineConfigurationParam:
            {
                SaveMachineConfigurationParamAction saveAction = static_cast<const SaveMachineConfigurationParamAction&>(action);
                QString command = QString("$%1=%2").arg(saveAction.index()).arg(saveAction.value());
                qDebug() << "[IdleBehavior] Saving machine configuration parameter:" << command;
                m_communicator->sendCommand(CommandSource::System, command);
            }
            return true;

        case Action::Type::Home:
            emit transition(this, new HomingBehavior());
            return true;

        case Action::Type::Run:
            {
                RunAction runAction = static_cast<const RunAction&>(action);
                emit transition(this, new RunningBehavior(runAction.program()));
            }
            return true;

        case Action::Type::Probe:
            emit transition(this, new ProbingBehavior());
            return true;
    }

    return StateBehavior::doAction(action);
}

StateBehavior::Result IdleBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[IdleBehavior] Entry";

    communicator->startQueryingMachineState();

    return StateBehavior::onEntry(communicator, previous);
}
