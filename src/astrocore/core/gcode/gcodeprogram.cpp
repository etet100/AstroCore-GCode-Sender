// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeprogram.h"
#include <QCryptographicHash>

void GCodeProgram::clear()
{
    m_data.clear();
    m_responses.clear();
    m_mainCount = 0;
}

int GCodeProgram::append(const GCodeItem& item)
{
    if (item.overlayId == 0) m_mainCount++;
    m_data.append(item);
    m_data.last().lineNumber = m_data.count();

    return m_data.count() - 1;
}

int GCodeProgram::append(GCodeItem&& item)
{
    if (item.overlayId == 0) m_mainCount++;
    m_data.append(std::move(item));
    m_data.last().lineNumber = m_data.count();

    return m_data.count() - 1;
}

void GCodeProgram::appendProgram(const GCodeProgram& source)
{
    for (const GCodeItem& item : source.m_data) {
        if (item.overlayId == 0) m_mainCount++;
        m_data.append(item);
    }
}

void GCodeProgram::insert(int index, const GCodeItem& item)
{
    if (item.overlayId == 0) m_mainCount++;
    m_data.insert(index, item);
    // Structural edits invalidate index-based response keys, so drop the map.
    m_responses.clear();
}

void GCodeProgram::removeAt(int index)
{
    if (m_data[index].overlayId == 0) m_mainCount--;
    m_data.removeAt(index);
    m_responses.clear();
}

void GCodeProgram::erase(int begin, int end)
{
    for (int i = begin; i < end; i++) {
        if (m_data[i].overlayId == 0) m_mainCount--;
    }
    m_data.erase(m_data.begin() + begin, m_data.begin() + end);
    m_responses.clear();
}

void GCodeProgram::deleteLines(int from, int to)
{
    for (int i = from; i <= to; i++) {
        if (m_data[i].overlayId == 0) m_mainCount--;
    }
    m_data.remove(from, to - from + 1);
    // Structural edit — index-based response keys are no longer valid.
    m_responses.clear();
}

bool GCodeProgram::replace(int from, int to, const QList<GCodeItem>& items)
{
    for (int i = from; i <= to; i++) {
        if (m_data[i].overlayId == 0) m_mainCount--;
    }
    m_data.remove(from, to - from + 1);

    int i = 0;
    for (const GCodeItem& item : items) {
        if (item.overlayId == 0) m_mainCount++;
        m_data.insert(from + i, item);
        i++;
    }
    m_responses.clear();

    return items.count() == (to - from + 1);
}

QString GCodeProgram::linesAsText(int from, int to) const
{
    if (to >= m_data.size()) {
        to = m_data.size() - 1;
    }
    if (from < 0) {
        from = 0;
    }
    if (from > to) {
        return QString();
    }

    QStringList lines;

    for (QList<GCodeItem>::const_iterator it = m_data.begin() + from; it != m_data.begin() + to + 1; ++it) {
        lines.append(it->line);
    }

    return lines.join("\n");
}

void GCodeProgram::resetItemStates()
{
    for (auto& item : m_data) {
        // Is it good idea to rely on group here?
        item.state = item.group == GCodeItemGroup::Comment ? GCodeItem::Comment : GCodeItem::InQueue;
    }
}

int GCodeProgram::insertOverlay(const QString& name, const QList<GCodeItem>& commands, int insertAt)
{
    int overlayId = ++m_nextOverlayId;

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

    return overlayId;
}

const OverlayInfo* GCodeProgram::overlayInfo(int overlayId) const
{
    auto it = m_overlays.constFind(overlayId);
    if (it == m_overlays.constEnd()) {
        return nullptr;
    }

    return &it.value();
}

bool GCodeProgram::resetOverlays(int fromIndex)
{
    if (m_overlays.isEmpty()) {
        return false;
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
        return false;
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

    return true;
}

bool GCodeProgram::isOverlayItem(int index) const
{
    if (index < 0 || index >= m_data.count()) {
        return false;
    }

    return m_data[index].overlayId > 0;
}

QString GCodeProgram::response(int index) const
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

void GCodeProgram::setResponse(int index, const QString& response)
{
    // Skip storing common "ok" / empty responses — they are inferred from state.
    if (response.isEmpty() || response == QStringLiteral("ok")) {
        m_responses.remove(index);
        return;
    }
    m_responses.insert(index, response);
}

QString GCodeProgram::calculateHash() const
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

void GCodeProgram::updateHash()
{
    m_contentHash = calculateHash();
}

bool GCodeProgram::isModified() const
{
    return m_contentHash != calculateHash();
}

void GCodeProgram::markAsNotModified()
{
    m_contentHash = calculateHash();
}
