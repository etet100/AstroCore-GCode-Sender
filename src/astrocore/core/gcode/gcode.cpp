// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcode.h"

GCode::GCode(QObject *parent) : QObject(parent)
{
    reset();

    m_linesUpdatedTimer.setInterval(100);
    m_linesUpdatedTimer.start();

    connect(&m_linesUpdatedTimer, &QTimer::timeout, this, &GCode::onLinesUpdatedTimer);
}

void GCode::reset(int commandIndex)
{
    m_cursor.reset(commandIndex);

    if (m_program.empty()) {
        return;
    }

    m_program.resetOverlays(commandIndex);
    m_program.clearResponses();
    m_program.resetItemStates();

    // Notify that all lines have been updated
    addUpdatedRange(0, m_program.count() - 1);
}

void GCode::resetProcessed(int commandIndex)
{
    m_cursor.resetProcessed(commandIndex);
}

QString GCode::command()
{
    return m_program[m_cursor.commandIndex()].command();
}

void GCode::advanceCommandIndex()
{
    int prevOverlayId = m_cursor.overlayIdAt(m_cursor.commandIndex());

    // Force update previous line and current line to update their states in the view (e.g. Sent -> Processed)
    addUpdatedRange(m_cursor.commandIndex());
    m_cursor.advance();
    addUpdatedRange(m_cursor.commandIndex());

    int nextOverlayId = m_cursor.overlayIdAt(m_cursor.commandIndex());

    if (nextOverlayId != prevOverlayId) {
        emit activeOverlayChanged(nextOverlayId);
    }
}

void GCode::addUpdatedRange(int index1, int index2)
{
    m_linesUpdatedFrom = qMin(m_linesUpdatedFrom, index1);
    m_linesUpdatedTo = qMax(qMax(m_linesUpdatedTo, index1), index2);
}

void GCode::setCommandSent()
{
    int index = m_cursor.commandIndex();
    m_program[index].state = GCodeItem::Sent;
    addUpdatedRange(index);
    m_lastSentCommand = index;
}

void GCode::setCommandResponse(int commandIndex, bool success, QString response)
{
    m_program[commandIndex].state = success ? GCodeItem::Processed : GCodeItem::Error;
    m_program.setResponse(commandIndex, response);
    m_cursor.setProcessedIndex(commandIndex);
    addUpdatedRange(commandIndex);
}

void GCode::setCommandAborted(int commandIndex)
{
    m_program[commandIndex].state = GCodeItem::Aborted;
    addUpdatedRange(commandIndex);
}

void GCode::setCommandSkipped()
{
    int index = m_cursor.commandIndex();
    m_program[index].state = GCodeItem::Skipped;
    addUpdatedRange(index);
}

void GCode::deleteLines(int from, int to)
{
    m_program.deleteLines(from, to);

    // Row count and all indices after 'from' change, so views must reset.
    emit structureChanged();
}

void GCode::replace(int from, int to, GCode &gcode)
{
    bool sameLineCount = m_program.replace(from, to, gcode.m_program.items());

    if (sameLineCount) {
        // If replacement has the same number of lines as removed block, only those lines are updated
        addUpdatedRange(from, to);
    } else {
        // Line count changed, so row count and indices after 'from' are no longer valid.
        emit structureChanged();
    }
}

GCode &GCode::operator <<(const GCode &source)
{
    m_program.appendProgram(source.m_program);
    emit loaded();

    return *this;
}

GCode &GCode::operator <<(GCodeItem &&item)
{
    int index = m_program.append(std::move(item));
    emit linesUpdated(index, index);

    return *this;
}

GCode &GCode::operator <<(const GCodeItem &item)
{
    int index = m_program.append(item);
    emit linesUpdated(index, index);

    return *this;
}

int GCode::insertOverlay(const QString& name, const QList<GCodeItem>& commands)
{
    if (commands.isEmpty()) {
        return 0;
    }

    // Only one overlay level allowed - reject if current command is already an overlay
    if (m_cursor.iterationStarted() && m_cursor.commandIndex() < m_program.count()
        && m_program[m_cursor.commandIndex()].overlayId > 0) {

        return 0;
    }

    int insertAt = m_cursor.iterationStarted() ? m_cursor.commandIndex() + 1 : m_cursor.commandIndex();
    int overlayId = m_program.insertOverlay(name, commands, insertAt);

    addUpdatedRange(insertAt);
    addUpdatedRange(m_program.count() - 1);

    return overlayId;
}

void GCode::resetOverlays(int fromIndex)
{
    if (!m_program.resetOverlays(fromIndex)) {
        return;
    }

    addUpdatedRange(fromIndex);
    if (!m_program.empty()) {
        addUpdatedRange(m_program.count() - 1);
    }
}

void GCode::onLinesUpdatedTimer()
{
    flushPendingUpdates();
}

void GCode::flushPendingUpdates()
{
    if (m_lastSentCommand != INT_MAX) {
        emit lastSentCommandChanged(m_lastSentCommand);
        m_lastSentCommand = INT_MAX;
    }

    if (m_linesUpdatedTo != INT_MIN) {
        emit linesUpdated(m_linesUpdatedFrom, m_linesUpdatedTo);

        m_linesUpdatedFrom = INT_MAX;
        m_linesUpdatedTo = INT_MIN;
    }
}

void GCode::setAutoFlushEnabled(bool enabled)
{
    if (enabled) {
        m_linesUpdatedTimer.start();
    } else {
        m_linesUpdatedTimer.stop();
    }
}
