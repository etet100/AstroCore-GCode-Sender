// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcode.h"
#include <QCryptographicHash>

QString GCodeItem::command() const
{
    if (line.isEmpty()) {
        return QString();
    }

    // Truncate at ';' (rest of line is a comment), then remove every '(...)'
    // block. Per NIST RS-274 G-code allows inline parenthesised comments,
    // e.g. "G1 (ostroznie) X10 (feed) Y20" => "G1 X10 Y20".
    const int semiPos = line.indexOf(';');
    QString cmd = (semiPos >= 0) ? line.left(semiPos) : line;

    int open;
    while ((open = cmd.indexOf('(')) >= 0) {
        const int close = cmd.indexOf(')', open);
        if (close < 0) {
            cmd.truncate(open);
            break;
        }
        cmd.remove(open, close - open + 1);
    }
    return cmd.trimmed().toUpper();
}

bool GCodeItem::isArc() const
{
    return group == GCodeItemGroup::ArcMovement;
}

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
    m_iterationStarted = false;

    if (m_data.empty()) {
        return;
    }

    resetOverlays(commandIndex);

    m_responses.clear();
    for (auto& item : m_data) {
        // Is it good idea to rely on group here?
        item.state = item.group == GCodeItemGroup::Comment ? GCodeItem::Comment : GCodeItem::InQueue;
    }

    // Notify that all lines have been updated
    addUpdatedRange(0, m_data.count() - 1);
}

void GCode::resetProcessed(int commandIndex)
{
    m_processedCommandIndex = commandIndex;
}

QString GCode::command()
{
    return m_data[m_commandIndex].command();
}

void GCode::advanceCommandIndex()
{
    m_iterationStarted = true;

    int prevOverlayId = m_commandIndex < m_data.count() ? m_data[m_commandIndex].overlayId : 0;

    // Force update previous line and current line to update their states in the view (e.g. Sent -> Processed)
    m_linesUpdatedFrom = qMin(m_linesUpdatedFrom, m_commandIndex);
    m_commandIndex++;
    m_linesUpdatedTo = qMax(m_linesUpdatedTo, m_commandIndex);

    int nextOverlayId = m_commandIndex < m_data.count() ? m_data[m_commandIndex].overlayId : 0;

    if (nextOverlayId != prevOverlayId) {
        emit activeOverlayChanged(nextOverlayId);
    }
}

bool GCode::isLastCommand()
{
    return m_commandIndex == m_data.count() - 1;
}

bool GCode::noMoreCommands()
{
    return m_commandIndex > m_data.count() - 1;
}

/*
 * Returns true if m_commandIndex is less or equal to the last item index
 * and there are items in the list, false otherwise
 *
 * If true, command can be safe accessed by command() method
 */
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

void GCode::addUpdatedRange(int index1, int index2)
{
    m_linesUpdatedFrom = qMin(m_linesUpdatedFrom, index1);
    m_linesUpdatedTo = qMax(qMax(m_linesUpdatedTo, index1), index2);
}

QString GCode::calculateHash() const
{
    QCryptographicHash hash(QCryptographicHash::Md5);
    for (const GCodeItem& item : m_data) {
        if (item.overlayId != 0) {
            continue; // skip overlay items
        }
        hash.addData(item.line.toUtf8());
    }

    return QLatin1String(hash.result().toHex());
}

void GCode::updateHash()
{
    m_contentHash = calculateHash();
}

bool GCode::isModified() const
{
    return m_contentHash != calculateHash();
}

void GCode::setCommandSent()
{
    GCodeItem& item = m_data[m_commandIndex];
    item.state = GCodeItem::Sent;
    addUpdatedRange(m_commandIndex);
    m_lastSentCommand = m_commandIndex;
}

void GCode::setCommandResponse(int commandIndex, bool success, QString response)
{
    GCodeItem& item = m_data[commandIndex];
    item.state = success ? GCodeItem::Processed : GCodeItem::Error;
    setResponse(commandIndex, response);
    m_processedCommandIndex = commandIndex;
    addUpdatedRange(commandIndex);
}

QString GCode::response(int index) const
{
    const auto it = m_responses.constFind(index);
    if (it != m_responses.constEnd()) {
        return it.value();
    }
    // "ok" is implicit for processed lines; everything else has no response.
    if (index >= 0 && index < m_data.count()
        && m_data.at(index).state == GCodeItem::Processed) {
        return QStringLiteral("ok");
    }
    return QString();
}

void GCode::setResponse(int index, const QString& response)
{
    // Skip storing common "ok" / empty responses — they are inferred from state.
    if (response.isEmpty() || response == QStringLiteral("ok")) {
        m_responses.remove(index);
        return;
    }
    m_responses.insert(index, response);
}

void GCode::clearResponses()
{
    m_responses.clear();
}

void GCode::setCommandAborted(int commandIndex)
{
    GCodeItem& item = m_data[commandIndex];
    item.state = GCodeItem::Aborted;
    addUpdatedRange(commandIndex);
}

void GCode::setCommandSkipped()
{
    GCodeItem& item = m_data[m_commandIndex];
    item.state = GCodeItem::Skipped;
    addUpdatedRange(m_commandIndex);
}

void GCode::deleteLines(int from, int to)
{
    for (int i = from; i <= to; i++) {
        if (m_data[i].overlayId == 0) m_mainCount--;
    }

    m_data.remove(from, to - from + 1);

    // All lines after 'to' are also updated because their indices have changed
    emit linesUpdated(from, m_data.count() - 1);
}

QString GCode::linesAsText(int from, int to)
{
    if (to >= m_data.size()) {
        to = m_data.size() - 1;
    }

    QStringList lines;

    for (QList<GCodeItem>::iterator it = m_data.begin() + from; it != m_data.begin() + to + 1; ++it) {
        lines.append(it->line);
    }

    return lines.join("\n");
}

void GCode::replace(int from, int to, GCode &gcode)
{
    for (int i = from; i <= to; i++) {
        if (m_data[i].overlayId == 0) m_mainCount--;
    }
    m_data.remove(from, to - from + 1);

    int i = 0;
    for (auto& item : gcode) {
        if (item.overlayId == 0) m_mainCount++;
        m_data.insert(from + i, item);
        i++;
    }

    if (gcode.count() == (to - from + 1)) {
        // If replacement has the same number of lines as removed block, only those lines are updated
        addUpdatedRange(from, to);
    } else {
        // All lines after 'from' are updated because their indices have changed
        addUpdatedRange(from, m_data.count() - 1);
    }

}

GCode &GCode::operator <<(const GCode &source) {
    for (const GCodeItem& item : source.m_data) {
        if (item.overlayId == 0) m_mainCount++;
        m_data.append(item);
    }
    emit loaded();

    return *this;
}

GCode &GCode::operator <<(GCodeItem &&item) {
    if (item.overlayId == 0) m_mainCount++;
    m_data.append(std::move(item));
    m_data.last().lineNumber = m_data.count();
    emit linesUpdated(m_data.count() - 1, m_data.count() - 1);

    return *this;
}

GCode &GCode::operator <<(const GCodeItem &item) {
    if (item.overlayId == 0) m_mainCount++;
    m_data.append(item);
    m_data.last().lineNumber = m_data.count();
    emit linesUpdated(m_data.count() - 1, m_data.count() - 1);

    return *this;
}

int GCode::insertOverlay(const QString& name, const QList<GCodeItem>& commands)
{
    if (commands.isEmpty()) {
        return 0;
    }

    // Only one overlay level allowed - reject if current command is already an overlay
    if (m_iterationStarted && m_commandIndex < m_data.count()
        && m_data[m_commandIndex].overlayId > 0) {

        return 0;
    }

    int overlayId = ++m_nextOverlayId;
    int insertAt = m_iterationStarted ? m_commandIndex + 1 : m_commandIndex;

    OverlayInfo info;
    info.id = overlayId;
    info.name = name;
    info.insertedAt = insertAt;
    info.count = commands.count();
    m_overlays.insert(overlayId, info);

    // Single resize + block move instead of N individual inserts
    int oldSize = m_data.count();
    int n = commands.count();
    m_data.resize(oldSize + n);

    // Shift tail block once
    for (int i = oldSize - 1; i >= insertAt; i--) {
        m_data[i + n] = std::move(m_data[i]);
    }

    // Copy overlay items into the gap
    for (int i = 0; i < n; i++) {
        m_data[insertAt + i] = commands[i];
        m_data[insertAt + i].overlayId = overlayId;
    }

    addUpdatedRange(insertAt);
    addUpdatedRange(m_data.count() - 1);

    return overlayId;
}

const OverlayInfo* GCode::overlayInfo(int overlayId) const
{
    auto it = m_overlays.constFind(overlayId);
    if (it == m_overlays.constEnd()) {
        return nullptr;
    }

    return &it.value();
}

void GCode::resetOverlays(int fromIndex)
{
    if (m_overlays.isEmpty()) {
        return;
    }

    // Compact overlay items from fromIndex onwards, keep items before fromIndex
    int write = fromIndex;
    bool changed = false;

    for (int read = fromIndex; read < m_data.count(); read++) {
        if (m_data[read].overlayId == 0) {
            if (write != read) {
                m_data[write] = std::move(m_data[read]);
                changed = true;
            }
            write++;
        } else {
            changed = true;
        }
    }

    if (!changed) {
        return;
    }

    m_data.resize(write);

    // Remove overlay registry entries that started at or after fromIndex
    for (auto it = m_overlays.begin(); it != m_overlays.end(); ) {
        if (it->insertedAt >= fromIndex) {
            it = m_overlays.erase(it);
        } else {
            ++it;
        }
    }

    if (m_overlays.isEmpty()) {
        m_nextOverlayId = 0;
    }

    addUpdatedRange(fromIndex);
    if (!m_data.isEmpty()) {
        addUpdatedRange(m_data.count() - 1);
    }
}

bool GCode::isOverlayItem(int index) const
{
    if (index < 0 || index >= m_data.count()) {
        return false;
    }
    return m_data[index].overlayId > 0;
}

int GCode::mainCount() const
{
    return m_mainCount;
}

void GCode::onLinesUpdatedTimer()
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
