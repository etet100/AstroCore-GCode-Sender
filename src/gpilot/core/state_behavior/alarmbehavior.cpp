// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "core/communicator/communicator.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"

AlarmBehavior::AlarmBehavior(int alarmCode, bool resumeAfterUnlock, QObject *parent)
    : AbstractStateBehavior{parent}
    , m_alarmCode(alarmCode)
    , m_resumeAfterUnlock(resumeAfterUnlock)
{
    if (alarmCode) {
        setAlarmMessage();
    }
}

QString AlarmBehavior::description() {
    if (!m_alarmCode) {
        return "Alarm";
    }

    return "Alarm: " + m_alarmMessage;
}

void AlarmBehavior::onMachineStateChanged(MachineState state)
{
    qDebug() << "[Behavior][Alarm] Device State Changed:" << static_cast<int>(state);
    // Handle device state changes
    if (state == MachineState::Idle) {
        if (m_resumeAfterUnlock) {
            qDebug() << "[Behavior][Alarm] Unlocked, resuming previous behavior";
            emit resumePrevious();
        } else {
            emit transition(this, new IdleBehavior());
        }
    } else {
        qWarning() << "[Behavior][Alarm] Unexpected machine state after unlock attempt:" << static_cast<int>(state);
    }
}

AbstractStateBehavior::Result AlarmBehavior::doOnEntry(CommunicatorApi *communicator, const EntryContext &ctx)
{
    qDebug() << "[Behavior][Alarm] Entry";

    // Can send alarm state query if the controller supports it
    // m_communicator->sendCommand(CommandSource::System, "$?", TABLE_INDEX_UI);

    return Result::Ok;
}

void AlarmBehavior::setAlarmMessage()
{
    m_alarmMessage = ALARMS.value(m_alarmCode, QString("Unknown (%1)").arg(m_alarmCode));
}

AbstractStateBehavior::Result AlarmBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    qDebug() << "[Behavior][Alarm] Command Response:" << command << response;
    // Handle command responses in alarm state
    // if (command == "$X") {  // Unlock command
    //     if (cmdStatus.ok !fullResponse.contains("error")) {
    //         // Unlock successful, go to idle state
    //         // emit transition(this, new IdleBehavior());
    //     } else {
    //         // Unlock failed, stay in alarm state
    //         // emit error(this, "Failed to unlock alarm: " + response.join(" "));
    //     }

    //     return Result::Ok;
    // }

    // if (command == "$$") {
    //     if (!cmdStatus.ok) {
    //         qDebug() << "[Behavior][Alarm] Error receiving device configuration.";

    //         return Result::Ok;
    //     }

    //     qDebug() << "[Behavior][Alarm] Processing device configuration.";
    //     m_communicator->processDeviceConfiguration(fullResponse);

    //     return Result::Ok;
    // }

    return Result::Unhandled;
}

// void AlarmBehavior::onConnectionStateChanged(ConnectionState state)
// {
//     qDebug() << "[Behavior][Alarm] AbstractConnection State Changed:" << static_cast<int>(state);
//     if (state != ConnectionState::Connected) {
//         // If connection is lost, we might want to transition to a different state
//         // For now, we don't do anything special
//     }
// }

void AlarmBehavior::unlock()
{
    m_communicator->sendCommand(CommandSource::GeneralUI, "$X", TABLE_INDEX_UI);
    m_communicator->queryMachineState();
}

bool AlarmBehavior::doAction(const Action &action)
{
    if (handleMachineConfigurationActions(action)) {
        return true;
    }

    switch (action.type()) {
        case Action::Type::Unlock:
            unlock();

            return true;
    }

    return AbstractStateBehavior::doAction(action);
}
