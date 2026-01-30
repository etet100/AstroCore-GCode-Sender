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

StateBehavior::Result ProbingBehavior::onCommandResponse(QString command, CommandAttributes commandAttributes, CmdStatus cmdStatus, QString response, QStringList fullResponse)
{
    qDebug() << "[ProbingBehavior] Command Response:" << command << response << fullResponse;

    if (m_alarm) {
        m_alarm = false;
        emit transition(this, new AlarmBehavior(m_alarmCode));

        log("Probing failed - alarm received during probing.", QStringList() << "Probing" );
    } else {
        log("Probing completed successfully.", QStringList() << "Probing" );

        emit transition(this, m_previous);
    }

    return StateBehavior::Result::Ok;
}

StateBehavior::Result ProbingBehavior::onEntry(CommunicatorApi *communicator, StateBehavior *previous)
{
    qDebug() << "[ProbingBehavior] Entry";
    StateBehavior::onEntry(communicator, previous);

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
