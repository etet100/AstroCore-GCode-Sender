// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include <QRegularExpression>
#include "runningbehavior.h"
#include "alarmbehavior.h"
#include "resetbehavior.h"
#include "core/communicator/communicator.h"

ResetBehavior::ResetBehavior(QObject *parent)
    : StateBehavior{parent}
{}

void ResetBehavior::onDeviceStateChanged(DeviceState state)
{
    qDebug() << "[ResetBehavior] Device State Changed:" << static_cast<int>(state);
    // // Handle device state changes
    if (state == DeviceState::Alarm) {
        emit transition(this, new AlarmBehavior());
    }
    //     // Machine started running - transition to running behavior
    //     emit transition(this, new RunningBehavior(this));
    // } else if (state == DeviceState::Alarm) {
    //     // Machine entered alarm state
    //     emit transition(this, new AlarmBehavior());
    // }
}

void ResetBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, QStringList response)
{
    qDebug() << "[ResetBehavior] Command Response:" << command << response;

    if (dataIsReset(response.first())) {
        qDebug() << "[ResetBehavior] Reset detected in response.";
        m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
        m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);

        return;
    }

    if (command == "$$") {
        qDebug() << "[ConnectingBehavior] Processing device configuration.";
        m_communicator->processDeviceConfiguration(response);

        return;
    }

    if (command == "$#") {
        qDebug() << "[ConnectingBehavior] Processing offsets.";
        m_communicator->processOffsetsVars(response.first());
        emit transition(this, new IdleBehavior());

        return;
    }

    // if (command == "$G") {
    //     m_communicator->processGCodeParserState(commandAttributes, response.first());
    // }
}

void ResetBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[ResetBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    QString command = "[CTRL+X]";
    CommandAttributes commandAttributes(
        CommandSource::System,
        m_communicator->m_commandIndex++,
        TABLE_INDEX_UI, // why UI ??
        command
    );
    m_communicator->m_commands.append(commandAttributes);
    m_communicator->connection()->sendByteArray(QByteArray(1, GRBL_LIVE_SOFT_RESET));

    // m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    // m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);
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
    static QRegularExpression re("^(GRBL|GCARVIN)\\s\\d\\.\\d.", QRegularExpression::CaseInsensitiveOption);

    return data.contains(re);
}
