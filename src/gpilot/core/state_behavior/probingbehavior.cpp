// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "probingbehavior.h"
#include "alarmbehavior.h"
#include "idlebehavior.h"
#include "core/communicator/communicator.h"
#include <QRegularExpression>

ProbingBehavior::ProbingBehavior(QObject* parent)
    : StateBehavior{parent}
    , m_params{}  // Uses default member initializers
    , m_stage(ProbeStage::InitialSetup)
    , m_alarmOccurred(false)
    , m_alarmCode(0)
    , m_success(false)
    , m_initialStateAbsolute(true)
{}

ProbingBehavior::ProbingBehavior(ProbeParameters params, QObject* parent)
    : StateBehavior{parent}
    , m_params(params)
    , m_stage(ProbeStage::InitialSetup)
    , m_alarmOccurred(false)
    , m_alarmCode(0)
    , m_success(false)
    , m_initialStateAbsolute(true)
{}

QString ProbingBehavior::description()
{
    return QString("Probing - %1").arg(stageDescription());
}

StateBehavior::Result ProbingBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[Behavior][Probing] Entry - starting two-phase probing sequence";
    StateBehavior::onEntry(communicator, previous);

    m_communicator->startQueryingMachineState();

    // Start with initial setup
    log("Starting probing sequence...", {"Probing"});

    // Setup: Switch to relative positioning, ensure metric units
    m_communicator->sendCommand(CommandSource::GeneralUI, "G91 G21", TABLE_INDEX_UI);
    m_stage = ProbeStage::InitialSetup;

    return StateBehavior::Result::Ok;
}

StateBehavior::Result ProbingBehavior::onExit(StateBehavior *next)
{
    Q_UNUSED(next);
    qDebug() << "[Behavior][Probing] Exit";

    m_communicator->stopQueryingMachineState();

    return StateBehavior::onExit(next);
}

void ProbingBehavior::onAlarm(int code)
{
    qDebug() << "[Behavior][Probing] Alarm received:" << code;

    m_alarmOccurred = true;
    m_alarmCode = code;

    // Check if it's a probe failure alarm
    if (code == GRBL_ALARM_PROBE_FAIL_1 || code == GRBL_ALARM_PROBE_FAIL_2) {
        log("Probe failed - no contact detected", {"Probing", "Error"});
        emit probeFailed("No contact detected during probing");
    } else {
        log(QString("Alarm during probing: %1").arg(code), {"Probing", "Error"});
        emit probeFailed(QString("Alarm %1").arg(code));
    }
}

void ProbingBehavior::onMachineStateChanged(MachineState state)
{
    qDebug() << "[Behavior][Probing] Machine state changed:" << static_cast<int>(state)
             << "Stage:" << static_cast<int>(m_stage);

    // If alarm occurred, transition to alarm state
    if (m_alarmOccurred) {
        emit transition(this, new AlarmBehavior(m_alarmCode));
        return;
    }

    // When machine becomes idle, we can proceed to next stage for movement commands
    if (state == MachineState::Idle) {
        // Some stages need to wait for idle state before proceeding
        // This is handled in onCommandResponse
    }
}

StateBehavior::Result ProbingBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes,
                                                          CmdStatus cmdStatus, QString response,
                                                          QStringList fullResponse)
{
    Q_UNUSED(commandAttributes);

    qDebug() << "[Behavior][Probing] Command Response:" << command << "->" << response
             << "Stage:" << static_cast<int>(m_stage);

    // If alarm occurred, stop processing
    if (m_alarmOccurred) {
        return StateBehavior::Result::Ok;
    }

    // Check for errors
    if (!cmdStatus.ok) {
        qDebug() << "[Behavior][Probing] Command error:" << cmdStatus.errorCode;
        log(QString("Probing command error: %1").arg(enrichErrorMessage(response)), {"Probing", "Error"});
        finishProbing(false);
        return StateBehavior::Result::Ok;
    }

    // Process based on current stage
    switch (m_stage) {
        case ProbeStage::InitialSetup:
            if (command.contains("G91")) {
                qDebug() << "[Behavior][Probing] Initial setup complete, starting fast probe";
                startFastProbe();
            }
            break;

        case ProbeStage::FastProbeWait:
            if (command.contains("G38.2")) {
                QVector3D position;
                bool contacted = false;

                if (parseProbeResponse(fullResponse, position, contacted)) {
                    if (contacted) {
                        m_fastProbePosition = position;
                        log(QString("Fast probe contact at Z=%1").arg(position.z(), 0, 'f', 3), {"Probing"});
                        qDebug() << "[Behavior][Probing] Fast probe successful, retracting...";
                        startRetract();
                    } else {
                        log("Fast probe failed - no contact detected", {"Probing", "Error"});
                        emit probeFailed("No contact during fast probe");
                        finishProbing(false);
                    }
                } else {
                    log("Failed to parse probe response", {"Probing", "Error"});
                    finishProbing(false);
                }
            }
            break;

        case ProbeStage::RetractWait:
            if (command.contains("G0") || command.contains("G1")) {
                qDebug() << "[Behavior][Probing] Retract complete, starting slow probe";
                if (m_params.doubleProbe) {
                    startSlowProbe();
                } else {
                    // If not doing double probe, use fast probe result as final
                    m_probedPosition = m_fastProbePosition;
                    m_success = true;
                    log("Probing completed (single probe mode)", {"Probing"});
                    emit probeCompleted(m_probedPosition);

                    if (m_params.setZeroAtProbe) {
                        setZeroPosition();
                    } else {
                        moveToSafePosition();
                    }
                }
            }
            break;

        case ProbeStage::SlowProbeWait:
            if (command.contains("G38.2")) {
                QVector3D position;
                bool contacted = false;

                if (parseProbeResponse(fullResponse, position, contacted)) {
                    if (contacted) {
                        m_probedPosition = position;
                        m_success = true;
                        log(QString("Precise probe contact at Z=%1").arg(position.z(), 0, 'f', 3), {"Probing"});
                        qDebug() << "[Behavior][Probing] Slow probe successful";
                        emit probeCompleted(m_probedPosition);

                        if (m_params.setZeroAtProbe) {
                            setZeroPosition();
                        } else {
                            moveToSafePosition();
                        }
                    } else {
                        log("Slow probe failed - no contact detected", {"Probing", "Error"});
                        emit probeFailed("No contact during slow probe");
                        finishProbing(false);
                    }
                } else {
                    log("Failed to parse slow probe response", {"Probing", "Error"});
                    finishProbing(false);
                }
            }
            break;

        case ProbeStage::SetZero:
            if (command.contains("G92")) {
                qDebug() << "[Behavior][Probing] Z zero set, moving to safe position";
                log("Z axis zeroed at probe position", {"Probing"});
                moveToSafePosition();
            }
            break;

        case ProbeStage::MoveToSafe:
            if (command.contains("G0") || command.contains("G1")) {
                qDebug() << "[Behavior][Probing] Moved to safe position";

                // Return to absolute positioning if needed
                if (m_params.useAbsolute) {
                    m_communicator->sendCommand(CommandSource::GeneralUI, "G90", TABLE_INDEX_UI);
                }

                m_stage = ProbeStage::Completed;
                log("Probing completed successfully", {"Probing"});
                finishProbing(true);
            }
            break;

        case ProbeStage::Completed:
            // Finalize
            if (m_params.useAbsolute && command.contains("G90")) {
                qDebug() << "[Behavior][Probing] Returned to absolute mode, transitioning back";
                transitionToPreviousState();
            }
            break;

        default:
            break;
    }

    return StateBehavior::Result::Ok;
}

void ProbingBehavior::startFastProbe()
{
    m_stage = ProbeStage::FastProbe;

    QString cmd = QString("G38.2 Z-%1 F%2")
        .arg(m_params.maxDistance, 0, 'f', 3)
        .arg(m_params.fastFeedRate, 0, 'f', 1);

    log(QString("Fast probe: %1").arg(cmd), {"Probing"});
    m_communicator->sendCommand(CommandSource::GeneralUI, cmd, TABLE_INDEX_UI);
    m_stage = ProbeStage::FastProbeWait;
}

void ProbingBehavior::startRetract()
{
    m_stage = ProbeStage::Retract;

    QString cmd = QString("G0 Z%1")
        .arg(m_params.retractDistance, 0, 'f', 3);

    log(QString("Retracting: %1mm").arg(m_params.retractDistance, 0, 'f', 3), {"Probing"});
    m_communicator->sendCommand(CommandSource::GeneralUI, cmd, TABLE_INDEX_UI);
    m_stage = ProbeStage::RetractWait;
}

void ProbingBehavior::startSlowProbe()
{
    m_stage = ProbeStage::SlowProbe;

    // Probe slightly more than retract distance to ensure contact
    double probeDistance = m_params.retractDistance + 1.0;
    QString cmd = QString("G38.2 Z-%1 F%2")
        .arg(probeDistance, 0, 'f', 3)
        .arg(m_params.slowFeedRate, 0, 'f', 1);

    log(QString("Slow probe: %1").arg(cmd), {"Probing"});
    m_communicator->sendCommand(CommandSource::GeneralUI, cmd, TABLE_INDEX_UI);
    m_stage = ProbeStage::SlowProbeWait;
}

void ProbingBehavior::setZeroPosition()
{
    m_stage = ProbeStage::SetZero;

    log("Setting Z=0 at probe position", {"Probing"});
    m_communicator->sendCommand(CommandSource::GeneralUI, "G92 Z0", TABLE_INDEX_UI);
}

void ProbingBehavior::moveToSafePosition()
{
    m_stage = ProbeStage::MoveToSafe;

    QString cmd = QString("G0 Z%1")
        .arg(m_params.safeDistance, 0, 'f', 3);

    log(QString("Moving to safe position: +%1mm").arg(m_params.safeDistance, 0, 'f', 3), {"Probing"});
    m_communicator->sendCommand(CommandSource::GeneralUI, cmd, TABLE_INDEX_UI);
}

bool ProbingBehavior::parseProbeResponse(const QStringList &fullResponse, QVector3D &position, bool &contacted)
{
    // Look for [PRB:x,y,z:success] in response
    // Example: [PRB:0.000,0.000,-8.530:1]
    // Last digit: 1 = contact, 0 = no contact

    static QRegularExpression probeRegex(R"(\[PRB:([\-\d\.]+),([\-\d\.]+),([\-\d\.]+):(\d)\])");

    for (const QString &line : fullResponse) {
        QRegularExpressionMatch match = probeRegex.match(line);
        if (match.hasMatch()) {
            position.setX(match.captured(1).toDouble());
            position.setY(match.captured(2).toDouble());
            position.setZ(match.captured(3).toDouble());
            contacted = (match.captured(4) == "1");

            qDebug() << "[Behavior][Probing] Parsed probe response:"
                     << "X=" << position.x()
                     << "Y=" << position.y()
                     << "Z=" << position.z()
                     << "Contacted=" << contacted;

            return true;
        }
    }

    qDebug() << "[Behavior][Probing] Failed to parse probe response:" << fullResponse;
    return false;
}

void ProbingBehavior::finishProbing(bool success)
{
    m_success = success;

    if (!success) {
        log("Probing sequence failed", {"Probing", "Error"});

        // Try to return to safe position even on failure
        m_communicator->sendCommand(CommandSource::GeneralUI,
                                   QString("G0 Z%1").arg(m_params.safeDistance),
                                   TABLE_INDEX_UI);
    }

    // Return to absolute if needed
    if (m_params.useAbsolute) {
        m_communicator->sendCommand(CommandSource::GeneralUI, "G90", TABLE_INDEX_UI);
    }

    // Transition back
    transitionToPreviousState();
}

QString ProbingBehavior::stageDescription() const
{
    switch (m_stage) {
        case ProbeStage::InitialSetup: return "Setup";
        case ProbeStage::FastProbe:
        case ProbeStage::FastProbeWait: return "Fast Probe";
        case ProbeStage::Retract:
        case ProbeStage::RetractWait: return "Retract";
        case ProbeStage::SlowProbe:
        case ProbeStage::SlowProbeWait: return "Slow Probe";
        case ProbeStage::SetZero: return "Set Zero";
        case ProbeStage::MoveToSafe: return "Move to Safe";
        case ProbeStage::Completed: return "Completed";
        default: return "Unknown";
    }
}
