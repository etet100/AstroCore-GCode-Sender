// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODETABLEMODEL_H
#define GCODETABLEMODEL_H

#include "core/gcode/gcode.h"
#include "core/gcode/gcodefilterview.h"
#include <QAbstractTableModel>
#include <QString>

enum class GCodeTableColumn {
    Number = 0,
    Command = 1,
    State,
    Response
};

class GCodeTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit GCodeTableModel(GCode* program, QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    void setProgram(GCode* program);
    bool insertRow(int row, const QModelIndex& parent = QModelIndex());
    bool removeRow(int row, const QModelIndex& parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    void clear();
    void update();
    void updateLines(int from, int to);

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void showComments();
    void hideComments();
    void setCommentsVisible(bool visible);

    void setFilter(const QString &text);
    void clearFilter();

    // Access to the underlying filter for advanced options (overlay visibility).
    GCodeFilterView* filter() { return &m_filter; }
    const GCodeFilterView* filter() const { return &m_filter; }

    // True when the filter currently hides at least one source row.
    bool isFilterActive() const { return m_filter.isActive(); }

    // View row -> source row. Pass-through when filter is inactive.
    // Returns -1 when the view row is invalid.
    int mapToSource(int viewRow) const;

    // Source row -> view row. Pass-through when filter is inactive.
    // For hidden source rows returns the nearest earlier visible view row
    // (matches GCodeFilterView::toViewRow). Returns -1 when invalid.
    int mapFromSource(int sourceRow) const;

private slots:
    void onFilterAboutToReset();
    void onFilterReset();
    void onFilterRangeChanged(int fromView, int toView);

private:
    GCode* m_data = nullptr;
    GCodeFilterView m_filter;
    QStringList m_headers;
};

#endif // GCODETABLEMODEL_H
