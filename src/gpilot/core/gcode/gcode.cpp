// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcode.h"

GCode::GCode(QObject *parent) : QObject(parent) {
    reset();

    m_linesUpdatedTimer.setInterval(100);
    m_linesUpdatedTimer.start();

    connect(&m_linesUpdatedTimer, &QTimer::timeout, this, &GCode::onLinesUpdatedTimer);
}

void GCode::reset(int commandIndex)
{
    m_commandIndex = commandIndex;
    m_processedCommandIndex = commandIndex;

    if (m_data.empty()) {
        return;
    }

    for (auto& item : m_data) {
        // Is it good idea to rely on group here?
        item.state = item.group == GCodeItemGroup::Comment ? GCodeItem::Comment : GCodeItem::InQueue;
        item.response.clear();
    }

    // Notify that all lines have been updated
    addUpdatedRange(0);
    addUpdatedRange(m_data.count() - 1);
}

void GCode::resetProcessed(int commandIndex)
{
    m_processedCommandIndex = commandIndex;
}

QString GCode::command()
{
    return m_data[m_commandIndex].command;
}

void GCode::advanceCommandIndex()
{
    m_commandIndex++;
}

bool GCode::isLastCommand()
{
    return m_commandIndex == m_data.count() - 1;
}

bool GCode::noMoreCommands()
{
    return m_commandIndex > m_data.count() - 1;
}

bool GCode::hasMoreCommands()
{
    return !m_data.empty() && m_commandIndex <= m_data.count() - 1;
}

int GCode::lastCommandIndex()
{
    return m_data.count() - 1;
}

bool GCode::isLastCommandProcessed()
{
    return m_processedCommandIndex == m_data.count() - 1;
}

void GCode::addUpdatedRange(int commandIndex)
{
    m_linesUpdatedFrom = qMin(m_linesUpdatedFrom, commandIndex);
    m_linesUpdatedTo = qMax(m_linesUpdatedTo, commandIndex);
}

void GCode::setCommandSent()
{
    GCodeItem& item = m_data[m_commandIndex];
    item.state = GCodeItem::Sent;
    addUpdatedRange(m_commandIndex);
}

void GCode::setCommandResponse(int commandIndex, bool success, QString response)
{
    GCodeItem& item = m_data[commandIndex];
    item.state = success ? GCodeItem::Processed : GCodeItem::Error;
    item.response = response;
    m_processedCommandIndex = commandIndex;
    addUpdatedRange(commandIndex);
}

void GCode::setCommandSkipped()
{
    GCodeItem& item = m_data[m_commandIndex];
    item.state = GCodeItem::Skipped;
    addUpdatedRange(m_commandIndex);
}

void GCode::onLinesUpdatedTimer()
{
    if (m_linesUpdatedTo == INT_MIN) {
        return;
    }

    emit linesUpdated(m_linesUpdatedFrom, m_linesUpdatedTo);

    m_linesUpdatedFrom = INT_MAX;
    m_linesUpdatedTo = INT_MIN;
}
