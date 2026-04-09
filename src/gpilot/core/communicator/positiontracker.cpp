#include "positiontracker.h"
#include "communicator.h"
#include "core/machine/physicalmachineconfiguration.h"
#include <QRegularExpression>
#include <QDebug>
#include <cassert>

PositionTracker::PositionTracker(Communicator* communicator)
    : QObject(nullptr)
    , m_communicator(communicator)
{}

void PositionTracker::reset()
{
    m_machinePos = QVector3D(0, 0, 0);
    m_workOffset = QVector3D(0, 0, 0);
}

bool PositionTracker::compareCoordinates(double x, double y, double z) const
{
    return m_machinePos.x() == x && m_machinePos.y() == y && m_machinePos.z() == z;
}

void PositionTracker::restoreOffsets(PhysicalMachineConfiguration* config)
{
    // Still have pre-reset working position — restore it after a reset.
    const QString units = config->unitsInches() ? "G20" : "G21";

    m_communicator->sendCommand(
        CommandSource::System,
        QString("%4G53G90X%1Y%2Z%3")
            .arg(m_machinePos.x()).arg(m_machinePos.y()).arg(m_machinePos.z()).arg(units),
        TABLE_INDEX_UTIL1
    );

    m_communicator->sendCommand(
        CommandSource::System,
        QString("%4G92X%1Y%2Z%3")
            .arg(m_workOffset.x()).arg(m_workOffset.y()).arg(m_workOffset.z()).arg(units),
        TABLE_INDEX_UTIL1
    );
}

void PositionTracker::processMachinePosition(const QString& line)
{
    static QRegularExpression re("([^,]*),([^,]*),([^,>|]*)");

    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
        QVector3D newPos(
            match.captured(1).toDouble(),
            match.captured(2).toDouble(),
            match.captured(3).toDouble()
        );
        if (newPos != m_machinePos) {
            m_machinePos = newPos;
            emit machinePosChanged(newPos);
        }
    }
}

// WPos is absolute — compute offset relative to machine pos.
void PositionTracker::processWorkPosition(const QString& line)
{
    static QRegularExpression re("([^,]*),([^,]*),([^,>|]*)");

    QRegularExpressionMatch match = re.match(line);
    if (match.hasMatch()) {
        QVector3D workPos(
            match.captured(1).toDouble(),
            match.captured(2).toDouble(),
            match.captured(3).toDouble()
        );

        QVector3D workOffset(
            m_machinePos.x() - workPos.x(),
            m_machinePos.y() - workPos.y(),
            m_machinePos.z() - workPos.z()
        );

        if (workOffset != m_workOffset) {
            m_workOffset = workOffset;
        }
    }
}

// WCO is the raw work coordinate offset from the machine.
void PositionTracker::processWorkOffset(const QString& line)
{
    static QRegularExpression re("([^,]*),([^,]*),([^,>|]*)");

    QRegularExpressionMatch match = re.match(line);
    if (!match.hasMatch()) return;

    QVector3D workOffset(
        match.captured(1).toDouble(),
        match.captured(2).toDouble(),
        match.captured(3).toDouble()
    );

    if (workOffset != m_workOffset) {
        m_workOffset = workOffset;
        m_coordCache.setCoords("W", m_workOffset);
    }
}

void PositionTracker::processNewToolPosition(bool isCheckMode, bool isLastCommandProcessed)
{
    if (!(isCheckMode && !isLastCommandProcessed)) {
        emit toolPositionReceived(m_machinePos - m_workOffset);
    }
}

// Parses the $# response — updates work coordinate offsets in MachineCoordinateCache.
void PositionTracker::processOffsetsVars(const QStringList& response)
{
    for (auto line : response) {
        if (line.startsWith('[') && line.endsWith(']')) {
            line = line.mid(1, line.length() - 2);
        } else {
            qDebug() << "[PositionTracker] Bad offsets line:" << line;
            assert(false);
            return;
        }

        QStringList parts = line.split(":");
        if (parts.size() != 2 && parts[0] != "PRB") {
            qDebug() << "[PositionTracker] Bad offsets format:" << line;
            assert(false);
            return;
        }

        QStringList axes = parts[1].split(",");
        QVector3D pos(
            axes.size() == 3 ? axes[0].toDouble() : 0,
            axes.size() == 3 ? axes[1].toDouble() : 0,
            axes.size() == 3 ? axes[2].toDouble() : axes[0].toDouble()
        );

        if (parts[0] == "G92") {
            qDebug() << "[PositionTracker] G92 offset updated";
            m_workOffset = pos;
        }

        m_coordCache.setCoords(parts[0], pos);
    }

    qDebug() << "[PositionTracker] Offsets updated";
}
