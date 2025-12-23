// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include <QRegularExpression>
#include "core/communicator/communicator.h"
#include "state_behaviour/behaviors.h"

ResetBehavior::ResetBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void ResetBehavior::onMachineState(MachineState state)
{
    if (m_stage != Completed) {
        return;
    }

    qDebug() << "[ResetBehavior] Device State:" << static_cast<int>(state);
    // // Handle device state changes
    if (state == MachineState::Idle) {
        emit transition(this, new IdleBehavior());

        return;
    } else if (state == MachineState::Alarm) {
        emit transition(this, new AlarmBehavior(m_communicator->lastAlarmCode()));

        return;
    } else {
        qDebug() << "[ResetBehavior] Unhandled state after reset:" << int(state);
    }
}

void ResetBehavior::onAlarm(int code)
{
    emit transition(this, new AlarmBehavior(code));
}

StateBehavior::Result ResetBehavior::onRawResponse(QString response)
{
    qDebug() << "[ResetBehavior] Raw Response:" << response;

    if (dataIsReset(response)) {
        if (m_stage == SentReset) {
            qDebug() << "[ResetBehavior] Reset detected in raw response. Sending $$.";

            m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);

            m_stage = SentSettingsAndOffsets;
        } else {
            // Ignore silently
        }

        return Result::Ok;
    }

    return Result::Unhandled;
}

StateBehavior::Result ResetBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);

    qDebug() << "[ResetBehavior] Command Response:" << command << response;

    if (command == "$$" && !cmdStatus.ok && cmdStatus.errorCode == 7) {
        qDebug() << "[ResetBehavior] Eeprom error during $$, requeue and wait for ok.";

        return Result::ReturnCommandToQueue;
    }
    if (command == "$#" && !cmdStatus.ok && cmdStatus.errorCode == 7) {
        qDebug() << "[ResetBehavior] Eeprom error during $#, requeue and wait for ok.";

        return Result::ReturnCommandToQueue;
    }

    // if (dataIsReset(response)) {
    //     qDebug() << "[ResetBehavior] Reset detected in response. Sending $$ and $#.";

    //     m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    //     m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);

    //     return true;
    // }

    if (command == "$$") {
        if (!cmdStatus.ok) {
            qDebug() << "[ResetBehavior] Error receiving device configuration.";

            return Result::Ok;
        }

        qDebug() << "[ResetBehavior] Processing device configuration.";
        m_communicator->processDeviceConfiguration(fullResponse);

        m_stage = ReceivedSettings;
        m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1);

        return Result::Ok;
    }

    if (command == "$#") {
        if (!cmdStatus.ok) {
            qDebug() << "[ResetBehavior] Error receiving offsets.";
            if (cmdStatus.errorCode != 7) {
                emit transition(this, new ErrorBehaviour(cmdStatus.errorCode));

                return Result::Ok;
            }

            qDebug() << "[ResetBehavior] We continue despite the error 7.";
        } else {
            qDebug() << "[ResetBehavior] Processing offsets.";
            m_communicator->processOffsetsVars(fullResponse);
        }

        qDebug() << "[ResetBehavior] Reset completed.";
        m_communicator->queryMachineState();

        // if (m_state == DeviceState::Alarm) {
        //     emit transition(this, new IdleBehavior());
        // } else {
        //     qDebug() << "[ConnectingBehavior] Unhandled state after reset:" << int(m_state);
        // }

        m_stage = Completed;

        return Result::Ok;
    }

    // if (command == "$G") {
    //     m_communicator->processGCodeParserState(commandAttributes, response.first());
    // }

    return Result::Unhandled;
}

StateBehavior::Result ResetBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[ResetBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    communicator->clearCommandsAndQueue();

    // QString command = "[CTRL+X]";
    // CommandAttributes commandAttributes(
    //     CommandSource::System,
    //     m_communicator->m_commandIndex++,
    //     TABLE_INDEX_UI, // why UI ??
    //     command
    // );
    // m_communicator->m_commands.append(commandAttributes);

    qDebug() << "[ResetBehavior] Soft reset";
    communicator->connection()->sendByteArray(QByteArray(1, GRBL_LIVE_SOFT_RESET));

    communicator->startQueryingMachineState();

    m_stage = SentReset;

    return Result::Ok;
}

bool ResetBehavior::dataIsReset(QString data)
{
    // "GRBL" in either case, optionally followed by a number of non-whitespace characters,
    // followed by a version number in the format x.y.
    // This matches e.g.
    // Grbl 1.1h ['$' for help]
    // GrblHAL 1.1f ['$' or '' for help]
    // Grbl 1.8 [uCNC v1.8.8 '$' for help]
    // Gcarvin ?? https://github.com/inventables/gCarvin
    static QRegularExpression re("^(GrblHAL|GRBL|GCARVIN)\\s\\d\\.\\d.", QRegularExpression::CaseInsensitiveOption);

    return data.contains(re);
}
