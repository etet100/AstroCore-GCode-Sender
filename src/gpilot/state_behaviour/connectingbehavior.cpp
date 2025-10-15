// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "connectingbehavior.h"
#include <QTimer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "core/communicator/communicator.h"

ConnectingBehavior::ConnectingBehavior(QObject *parent)
    : StateBehavior{parent}
{}

QString ConnectingBehavior::name() { return "Connecting"; }

void ConnectingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    qDebug() << "[ConnectingBehavior] Attempting to connect...";
    StateBehavior::onEntry(communicator, previous);

    if (communicator->connection()->open() && communicator->connection()->isConnected()) {
        return;
    }

    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    connect(m_timer, &QTimer::timeout, this, [this, communicator]() {
        qDebug() << "[ConnectingBehavior] Attempting to connect...";
        if (communicator->connection()->isConnected()) {
            stopTimer();

            return;
        }

        communicator->connection()->open();
    });
    m_timer->start();
}

void ConnectingBehavior::onExit(StateBehavior *next)
{
    StateBehavior::onExit(next);
}

void ConnectingBehavior::onConnectionStateChanged(ConnectionState state)
{
    if (state == ConnectionState::Connected) {
        stopTimer();
        qDebug() << "[ConnectingBehavior] Connected.";
        // m_communicator->reset();
        emit transition(this, new ResetBehavior());
    }
}

void ConnectingBehavior::onCommandResponse(QString command, QStringList response)
{
    qDebug() << "[ConnectingBehavior] Command Response:" << command << response;

    if (dataIsReset(response.first())) {
        qDebug() << "[ConnectingBehavior] Welcome message detected.";
        // m_communicator->reset();
        // // emit transition(this, new IdleBehavior());
        qDebug() << "[ConnectingBehavior] Sending $$ and $#";
        m_communicator->sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
        m_communicator->sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);
    }

    if (command == "$$") {
        qDebug() << "[ConnectingBehavior] Processing device configuration.";
        m_communicator->processDeviceConfiguration(response.first());
    }

    if (command == "$#") {
        qDebug() << "[ConnectingBehavior] Processing offsets.";
        m_communicator->processOffsetsVars(response.first());
        emit transition(this, new IdleBehavior());
    }


    // static QRegularExpression gs("\\$(\\d+)\\=([^;]+)\\; ");

        // QMap<int, double> rawMachineConfiguration;
        // int p = 0;
        // QRegularExpressionMatch match = gs.match(response);
        // while (match.hasMatch()) {
        //     rawMachineConfiguration[match.captured(1).toInt()] = match.captured(2).toDouble();
        //     p += match.capturedLength();
        //     match = gs.match(response, p);
        // }

        // MachineConfiguration *machineConfiguration = m_machineConfiguration = new MachineConfiguration(
        //     rawMachineConfiguration,
        //     m_configuration->machineModule()
        //     );

        // emit deviceConfigurationReceived(
        //     *machineConfiguration,
        //     rawMachineConfiguration
        //     );

        // if (commandAttributes.callback != nullptr) {
        //     commandAttributes.callback(machineConfiguration);
        // }

        // Command sent after reset
        // if (ca.tableIndex == -2) {
        //     QList<int> keys = rawMachineConfiguration.keys();
        //     if (keys.contains(13)) m_settings->setUnits(rawMachineConfiguration[13]);
        //     {...}

        //     //moved to settingsReceived signal handler
        //     //setupCoordsTextboxes();
        // }
    // }

    // // Homing response
    // if ((command == "$H" || command == "$T") && m_homing) m_homing = false;

    // // Reset complete response
    // if (command == "[CTRL+X]") {
    //     m_resetCompleted = true;
    //     m_updateParserState = true;

    //     // Query grbl settings
    //     sendCommand(CommandSource::System, "$$", TABLE_INDEX_UTIL1);
    //     sendCommand(CommandSource::System, "$#", TABLE_INDEX_UTIL1, true);
    // }
}

bool ConnectingBehavior::dataIsReset(QString data)
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
