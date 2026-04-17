// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#ifndef GCODEFILTERVIEW_H
#define GCODEFILTERVIEW_H

#include "gcode.h"
#include <QObject>
#include <QString>
#include <QList>
#include <QHash>

// Mediates between a GCode object and a view (e.g. GCodeTableModel).
// Hides rows based on filter settings (comments, text, overlays) and
// keeps a two-way mapping between source rows and visible view rows.
class GCodeFilterView : public QObject
{
    Q_OBJECT

public:
    explicit GCodeFilterView(QObject* parent = nullptr);

    void setSource(GCode* source);
    GCode* source() const { return m_source; }

    void setCommentsVisible(bool visible);
    bool commentsVisible() const { return m_showComments; }

    void setTextFilter(const QString& text);
    QString textFilter() const { return m_filterText; }

    // Per-overlay visibility override. Unset overlays use the default policy.
    void setOverlayVisible(int overlayId, bool visible);
    void clearOverlayOverride(int overlayId);
    void clearAllOverlayOverrides();

    // Default visibility for overlays without an explicit override.
    void setAllOverlaysVisible(bool visible);
    bool allOverlaysVisible() const { return m_defaultOverlayVisible; }

    bool isOverlayVisible(int overlayId) const;

    // True when any filter removes at least one source row.
    bool isActive() const { return m_active; }

    // Number of rows currently visible.
    int rowCount() const;

    // Pointer to the source item at the given view row, or nullptr if out of range.
    GCodeItem* at(int viewRow) const;

    // View row -> source row. Returns -1 if out of range.
    int toSourceRow(int viewRow) const;

    // Source row -> view row. If the given source row is hidden, returns the
    // nearest earlier visible view row (mirrors the legacy behavior used for
    // scrolling). Returns -1 if source is null or index is out of range.
    int toViewRow(int sourceRow) const;

signals:
    // Emitted when the mapping is about to change due to a filter change
    // or a new source. Views should pair this with a reset of their own.
    void aboutToReset();
    void reset();

    // Emitted when a range of source rows reported changes. Arguments are
    // already mapped to view coordinates. Views can treat this like
    // QAbstractItemModel::dataChanged for those rows.
    void rangeChanged(int fromViewRow, int toViewRow);

private slots:
    void onSourceLinesUpdated(int from, int to);

private:
    GCode* m_source = nullptr;
    bool m_showComments = true;
    QString m_filterText;
    bool m_defaultOverlayVisible = true;
    QHash<int, bool> m_overlayOverrides;

    bool m_active = false;
    QList<int> m_viewToSource;
    QList<int> m_sourceToView;  // for each source row: nearest <= visible view row

    void rebuild();
    bool passesFilter(const GCodeItem& item) const;
    void emitReset();
};

#endif // GCODEFILTERVIEW_H
