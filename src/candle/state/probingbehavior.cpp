// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "probingbehavior.h"
#include "../communicator.h"
#include "idlebehavior.h"
#include "alarmbehavior.h"
#include <QDebug>

ProbingBehavior::ProbingBehavior(StateBehavior *previous,
                              ProbeDirection direction,
                              double distance,
                              double feedRate,
                              QObject *parent)
    : StateBehavior{previous, parent}
    , m_direction(direction)
    , m_distance(distance)
    , m_feedRate(feedRate)
    , m_probeStarted(false)
    , m_probeCompleted(false)
    , m_probeResult(QVector3D(0, 0, 0))
{
}

void ProbingBehavior::onEntry(Communicator *communicator, StateBehavior *previous)
{
    StateBehavior::onEntry(communicator, previous);

    // Generate and send probing command
    QString probeCommand = generateProbeCommand();
    m_communicator->sendCommand(CommandSource::GeneralUI, probeCommand, TABLE_INDEX_UI);
    m_probeStarted = true;
    m_probeCompleted = false;
}

void ProbingBehavior::onDeviceStateChanged(DeviceState state)
{
    // Handle device state changes during probing
    if (state == DeviceState::Idle && m_probeStarted && !m_probeCompleted) {
        // Device went to idle state after probing started
        // This could mean the probe finished or was interrupted

        // We could do additional verification if probing was successful
        // For now we assume it was (full verification is in onCommandResponse)
    } else if (state == DeviceState::Alarm && m_probeStarted) {
        // Alarm occurred during probing - something likely went wrong
        emit error(this, "Probing failed - device entered alarm state");
        emit transition(this, new AlarmBehavior(this));
    }
}

void ProbingBehavior::onCommandResponse(QString command, QStringList response)
{
    qDebug() << "Probing response: " << response;

    // Check if response is for probing command (G38.2)
    if (command.contains("G38.2")) {
        if (response.contains("error")) {
            // Error occurred during probing command
            emit error(this, "Probing failed - " + response.join(" "));
        } else if (response.join(" ").contains("PRB:")) {
            // Found probing information in the response
            // Response format is usually [PRB:X,Y,Z:R] where X,Y,Z are coordinates and R is status (1=success)

            // Here we could add code to parse coordinates from the response
            // and save them in m_probeResult

            m_probeCompleted = true;

            // Could also emit a signal with probe result information
            // emit probeCompleted(m_probeResult);
        }

        // Regardless of result, after processing the response return to previous state
        if (m_previous) {
            emit transition(this, m_previous);
        } else {
            emit transition(this, new IdleBehavior(this));
        }
    }
}

void ProbingBehavior::setProbeParameters(ProbeDirection direction, double distance, double feedRate)
{
    m_direction = direction;
    m_distance = distance;
    m_feedRate = feedRate;
}

QString ProbingBehavior::generateProbeCommand()
{
    // Basic probing command
    QString command = "G38.2 ";

    // Add direction and distance
    switch (m_direction) {
        case ProbeDirection::ZMinus:
            command += "Z-" + QString::number(m_distance);
            break;
        case ProbeDirection::ZPlus:
            command += "Z" + QString::number(m_distance);
            break;
        case ProbeDirection::XMinus:
            command += "X-" + QString::number(m_distance);
            break;
        case ProbeDirection::XPlus:
            command += "X" + QString::number(m_distance);
            break;
        case ProbeDirection::YMinus:
            command += "Y-" + QString::number(m_distance);
            break;
        case ProbeDirection::YPlus:
            command += "Y" + QString::number(m_distance);
            break;
    }

    // Add feed rate
    command += " F" + QString::number(m_feedRate);

    return command;
}