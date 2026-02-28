#include "probingbehavior.h"
#include "alarmbehavior.h"
#include "core/communicator/communicator.h"

ProbingBehavior::ProbingBehavior(QObject* parent)
    : StateBehavior{parent}
{}

StateBehavior::Result ProbingBehavior::onExit(StateBehavior *next)
{
    qDebug() << "[ProbingBehavior] Exit";

    this->m_communicator->stopQueryingMachineState();

    return StateBehavior::Result::Ok;
}

void ProbingBehavior::onAlarm(int code)
{
    qDebug() << "[ProbingBehavior] Alarm received:" << code;

    m_alarm = true;
    m_alarmCode = code;
}

void ProbingBehavior::onMachineStateChanged(MachineState state)
{
    qDebug() << "[ProbingBehavior] Machine state changed:" << static_cast<int>(state);
}

StateBehavior::Result ProbingBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    qDebug() << "[ProbingBehavior] Command Response:" << command << response << fullResponse;

    if (m_alarm) {
        m_alarm = false;
        emit transition(this, new AlarmBehavior(m_alarmCode));

        qDebug() << "[ProbingBehavior] Probing failed - alarm received during probing.";
        log("Probing failed - alarm received during probing.", QStringList() << "Probing" );

        return StateBehavior::Result::Ok;
    }

    if (cmdStatus.ok && fullResponse.size() > 0) {
        log("Probing completed successfully.", QStringList() << "Probing" );

        // [PRB:0.000,0.000,-8.530:1]
        QString prbLine = fullResponse.first();
        prbLine = prbLine.mid(5, prbLine.length() - 6); // remove [PRB: and ]
        QStringList parts = prbLine.split(":");
        QStringList coords = parts[0].split(",");
        qDebug() << prbLine << parts << coords;
        if (coords.size() >= 3) {
            double x = coords[0].toDouble();
            double y = coords[1].toDouble();
            double z = coords[2].toDouble();

            qDebug() << "[ProbingBehavior] Probed coordinates:" << x << y << z;
            log(QString("Probed coordinates: X=%1 Y=%2 Z=%3").arg(x).arg(y).arg(z), QStringList() << "Probing" );
        }

        transitionToPreviousState();
    }

    return StateBehavior::Result::Ok;
}

StateBehavior::Result ProbingBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[ProbingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

    QStringList commands = QStringList({
        "G91 G21 G49", // Relative positioning, Metric units, Cancel offset
        "G38.2 Z-15 F150", // Probe downwards max 15mm at 150mm/min
        "G92 Z0", // Set current Z position as 0
        "G0 Z5", // Move probe up 5mm
        "G90", // Return to Absolute positioning
    });

    //CommandSource source, QString commandLine, int tableIndex = TABLE_INDEX_UI, bool wait = false
    this->m_communicator->sendCommand(CommandSource::GeneralUI, "G38.2 Z-10 F500");

    this->m_communicator->startQueryingMachineState();


    //     CommandSource::GeneralUI,
    //     QStringList({
    //         "G38.2 Z-10 F500",
    //         // "G91 G21 G38.2 Z-50 F100",
    //         // "G92 Z14.09",
    //         // "G0Z5 M30"
    //     }),
    //     TABLE_INDEX_UI



    return StateBehavior::Result::Ok;
}
