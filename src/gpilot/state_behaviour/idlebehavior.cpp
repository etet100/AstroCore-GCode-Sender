// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "runningbehavior.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"
#include "core/communicator/communicator.h"

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

bool IdleBehavior::action(const Action &action)
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
            m_communicator->home();
            return true;
    }

    return StateBehavior::action(action);
}

StateBehavior::Result IdleBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[IdleBehavior] Entry";
    return StateBehavior::onEntry(communicator, previous);
}
