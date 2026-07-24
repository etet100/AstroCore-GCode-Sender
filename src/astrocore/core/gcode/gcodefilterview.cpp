// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "gcodefilterview.h"

GCodeFilterView::GCodeFilterView(QObject* parent)
    : QObject(parent)
{
}

void GCodeFilterView::setSource(GCode* source)
{
    if (m_source == source) {
        return;
    }

    emit aboutToReset();

    if (m_source) {
        disconnect(m_source, &GCode::linesUpdated, this, &GCodeFilterView::onSourceLinesUpdated);
        disconnect(m_source, &GCode::structureChanged, this, &GCodeFilterView::onSourceStructureChanged);
    }

    m_source = source;

    if (m_source) {
        connect(m_source, &GCode::linesUpdated, this, &GCodeFilterView::onSourceLinesUpdated, Qt::UniqueConnection);
        connect(m_source, &GCode::structureChanged, this, &GCodeFilterView::onSourceStructureChanged, Qt::UniqueConnection);
    }

    rebuild();
    emit reset();
}

void GCodeFilterView::setCommentsVisible(bool visible)
{
    if (m_showComments == visible) {
        return;
    }
    m_showComments = visible;
    emitReset();
}

void GCodeFilterView::setTextFilter(const QString& text)
{
    QString trimmed = text.trimmed();
    if (m_filterText == trimmed) {
        return;
    }
    m_filterText = trimmed;
    emitReset();
}

void GCodeFilterView::setOverlayVisible(int overlayId, bool visible)
{
    const auto it = m_overlayOverrides.constFind(overlayId);
    if (it != m_overlayOverrides.constEnd() && it.value() == visible) {
        return;
    }
    m_overlayOverrides.insert(overlayId, visible);
    emitReset();
}

void GCodeFilterView::clearOverlayOverride(int overlayId)
{
    if (m_overlayOverrides.remove(overlayId) > 0) {
        emitReset();
    }
}

void GCodeFilterView::clearAllOverlayOverrides()
{
    if (m_overlayOverrides.isEmpty()) {
        return;
    }
    m_overlayOverrides.clear();
    emitReset();
}

void GCodeFilterView::setAllOverlaysVisible(bool visible)
{
    if (m_defaultOverlayVisible == visible) {
        return;
    }
    m_defaultOverlayVisible = visible;
    emitReset();
}

bool GCodeFilterView::isOverlayVisible(int overlayId) const
{
    if (overlayId <= 0) {
        return true;
    }
    const auto it = m_overlayOverrides.constFind(overlayId);
    if (it != m_overlayOverrides.constEnd()) {
        return it.value();
    }
    return m_defaultOverlayVisible;
}

int GCodeFilterView::rowCount() const
{
    if (!m_source) {
        return 0;
    }
    return m_active ? m_viewToSource.size() : m_source->count();
}

GCodeItem* GCodeFilterView::at(int viewRow) const
{
    const int sourceRow = toSourceRow(viewRow);
    if (sourceRow < 0 || !m_source) {
        return nullptr;
    }
    return &(*m_source)[sourceRow];
}

int GCodeFilterView::toSourceRow(int viewRow) const
{
    if (!m_source || viewRow < 0) {
        return -1;
    }
    if (!m_active) {
        return viewRow < m_source->count() ? viewRow : -1;
    }
    if (viewRow >= m_viewToSource.size()) {
        return -1;
    }
    return m_viewToSource.at(viewRow);
}

int GCodeFilterView::toViewRow(int sourceRow) const
{
    if (!m_source || sourceRow < 0) {
        return -1;
    }
    if (!m_active) {
        return sourceRow < m_source->count() ? sourceRow : -1;
    }
    if (sourceRow >= m_sourceToView.size()) {
        return -1;
    }
    return m_sourceToView.at(sourceRow);
}

void GCodeFilterView::onSourceLinesUpdated(int from, int to)
{
    // Structural changes are not reflected automatically in the mapping;
    // callers that change the source size must trigger a reset explicitly.
    // This matches the legacy GCodeTableModel behavior for linesUpdated.
    const int fromView = toViewRow(from);
    const int toView = toViewRow(to);
    if (fromView < 0 || toView < 0) {
        return;
    }
    emit rangeChanged(fromView, toView);
}

void GCodeFilterView::onSourceStructureChanged()
{
    // Line count changed, so the mapping and the view row count are stale.
    // A full reset rebuilds the mapping and refreshes the attached model.
    emitReset();
}

void GCodeFilterView::rebuild()
{
    m_viewToSource.clear();
    m_sourceToView.clear();

    if (!m_source) {
        m_active = false;
        return;
    }

    const bool overlayFilterActive =
        !m_defaultOverlayVisible
        || !m_overlayOverrides.isEmpty();

    m_active = !m_showComments || !m_filterText.isEmpty() || overlayFilterActive;

    if (!m_active) {
        return;
    }

    const int count = m_source->count();
    m_viewToSource.reserve(count);
    m_sourceToView.reserve(count);

    int lastViewIndex = 0;
    for (int i = 0; i < count; ++i) {
        const GCodeItem& row = m_source->at(i);
        if (passesFilter(row)) {
            lastViewIndex = m_viewToSource.size();
            m_viewToSource.append(i);
        }
        m_sourceToView.append(lastViewIndex);
    }
}

bool GCodeFilterView::passesFilter(const GCodeItem& item) const
{
    if (!m_showComments && item.group == GCodeItemGroup::Comment) {
        return false;
    }

    if (item.overlayId > 0 && !isOverlayVisible(item.overlayId)) {
        return false;
    }

    if (!m_filterText.isEmpty()) {
        const bool inCommand = item.command().contains(m_filterText, Qt::CaseInsensitive);
        const bool inComment = m_showComments && item.comment.contains(m_filterText, Qt::CaseInsensitive);
        if (!inCommand && !inComment) {
            return false;
        }
    }

    return true;
}

void GCodeFilterView::emitReset()
{
    emit aboutToReset();
    rebuild();
    emit reset();
}
