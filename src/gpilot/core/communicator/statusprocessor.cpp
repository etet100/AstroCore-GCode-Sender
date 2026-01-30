// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "statusprocessor.h"
#include <QRegularExpression>
#include <QDebug>

StatusProcessor::StatusProcessor(QObject *parent)
    : QObject(parent)
{
    m_machineStateDictionary = {
        {"Idle", MachineState::Idle},
        {"Run", MachineState::Run},
        {"Hold", MachineState::Hold0},
        {"Hold:0", MachineState::Hold0},
        {"Hold:1", MachineState::Hold1},
        {"Jog", MachineState::Jog},
        {"Alarm", MachineState::Alarm},
        {"Door", MachineState::Door0},
        {"Door:0", MachineState::Door0},
        {"Door:1", MachineState::Door1},
        {"Door:2", MachineState::Door2},
        {"Door:3", MachineState::Door3},
        {"Check", MachineState::Check},
        {"Home", MachineState::Home},
        {"Sleep", MachineState::Sleep},
        {"Queue", MachineState::Queue}
    };
}

MachineStatus StatusProcessor::parse(const QString &statusLine)
{
    MachineStatus status;
    status.rawLine = statusLine;

    // Remove < and >, split by |
    // <Run|MPos:-10.780,-9.740,3.000|Bf:0,932|FS:673,1000|WCO:0.000,0.000,0.000>
    if (!statusLine.startsWith('<') || !statusLine.endsWith('>')) {
        qDebug() << "[StatusProcessor] Invalid status line format:" << statusLine;
        return status;
    }

    QStringList sections = statusLine.mid(1, statusLine.length() - 2).split("|");

    if (sections.isEmpty()) {
        qDebug() << "[StatusProcessor] Empty status line";
        return status;
    }

    // First section is always machine state
    parseMachineState(sections.takeFirst(), status);

    // Process remaining sections
    for (QString section : sections) {
        if (section.startsWith("MPos:")) {
            parseMachinePosition(section.mid(5), status);
        } else if (section.startsWith("WPos:")) {
            parseWorkPosition(section.mid(5), status);
        } else if (section.startsWith("WCO:")) {
            parseWorkOffset(section.mid(4), status);
        } else if (section.startsWith("Ov:")) {
            parseOverrides(section.mid(3), status);
        } else if (section.startsWith("FS:")) {
            parseFeedSpindleSpeed(section.mid(3), status);
        } else if (section.startsWith("Bf:")) {
            parseBuffersStatus(section.mid(3), status);
        } else if (section.startsWith("Pn:")) {
            parsePinsState(section.mid(3), status);
        } else if (section.startsWith("A:")) {
            parseAccessoryState(section.mid(2), status);
        } else if (section.startsWith("H:")) {
            // Hold state - currently not processed
            qDebug() << "[StatusProcessor] Unhandled section H:" << section;
        } else {
            qDebug() << "[StatusProcessor] Unknown status section:" << section;
        }
    }

    return status;
}

void StatusProcessor::parseMachineState(const QString &stateStr, MachineStatus &status)
{
    status.state = m_machineStateDictionary.value(stateStr, MachineState::Unknown);
}

void StatusProcessor::parseMachinePosition(const QString &line, MachineStatus &status)
{
    static QRegularExpression mpx("([^,]*),([^,]*),([^,^>^|]*)");

    QRegularExpressionMatch match = mpx.match(line);
    if (match.hasMatch()) {
        status.machinePos = QVector3D(
            match.captured(1).toDouble(),
            match.captured(2).toDouble(),
            match.captured(3).toDouble()
        );
        status.hasMachinePos = true;
    }
}

void StatusProcessor::parseWorkPosition(const QString &line, MachineStatus &status)
{
    static QRegularExpression wpx("([^,]*),([^,]*),([^,^>^|]*)");

    QRegularExpressionMatch match = wpx.match(line);
    if (match.hasMatch()) {
        status.workPos = QVector3D(
            match.captured(1).toDouble(),
            match.captured(2).toDouble(),
            match.captured(3).toDouble()
        );
        status.hasWorkPos = true;
    }
}

void StatusProcessor::parseWorkOffset(const QString &line, MachineStatus &status)
{
    static QRegularExpression wpx("([^,]*),([^,]*),([^,^>^|]*)");

    QRegularExpressionMatch match = wpx.match(line);
    if (match.hasMatch()) {
        status.workOffset = QVector3D(
            match.captured(1).toDouble(),
            match.captured(2).toDouble(),
            match.captured(3).toDouble()
        );
        status.hasWorkOffset = true;
    }
}

void StatusProcessor::parseOverrides(const QString &line, MachineStatus &status)
{
    static QRegularExpression ov("([^,]*),([^,]*),([^,^>^|]*)");

    QRegularExpressionMatch match = ov.match(line);
    if (match.hasMatch()) {
        status.feedOverride = match.captured(1).toInt();
        status.rapidOverride = match.captured(2).toInt();
        status.spindleOverride = match.captured(3).toInt();
        status.hasOverrides = true;
    }
}

void StatusProcessor::parseFeedSpindleSpeed(const QString &line, MachineStatus &status)
{
    static QRegularExpression fs("([^,]*),([^,^|^>]*)");

    QRegularExpressionMatch match = fs.match(line);
    if (match.hasMatch()) {
        status.feedRate = match.captured(1).toInt();
        status.spindleSpeed = match.captured(2).toInt();
        status.hasFeedSpindleSpeed = true;
    }
}

void StatusProcessor::parseBuffersStatus(const QString &line, MachineStatus &status)
{
    static QRegularExpression fs(R"((\d*),(\d*))");

    QRegularExpressionMatch match = fs.match(line);
    if (match.hasMatch()) {
        status.bufferAvailable = match.captured(1).toInt();
        status.bufferSize = match.captured(2).toInt();
        status.hasBufferStatus = true;
    }
}

void StatusProcessor::parsePinsState(const QString &line, MachineStatus &status)
{
    status.pinStates = line;
    status.hasPinStates = true;
}

void StatusProcessor::parseAccessoryState(const QString &line, MachineStatus &status)
{
    status.spindleCW = line.contains("S");
    status.spindleEnabled = line.contains("S") || line.contains("C");
    status.floodEnabled = line.contains("F");
    status.mistEnabled = line.contains("M");
    status.hasAccessoryState = true;
}
