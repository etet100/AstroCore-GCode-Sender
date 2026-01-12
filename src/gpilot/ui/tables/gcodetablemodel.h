// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODETABLEMODEL_H
#define GCODETABLEMODEL_H

#include "core/gcode/gcode.h"
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
    explicit GCodeTableModel(GCode* data, QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole);
    void setProgram(GCode* data);
    bool insertRow(int row, const QModelIndex& parent = QModelIndex());
    bool removeRow(int row, const QModelIndex& parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    void clear();
    void update();

    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    void setCommentsVisible(bool visible);
    int toFilteredIndex(int index) const;

private slots:
    void notifyLinesUpdated(int fromLine, int toLine);

private:
    GCode* m_data;
    QStringList m_headers;
    bool m_filtered = false;
    QList<int> m_filteredRows; // index of rows that are not comments
    QList<int> m_allRowsToFiltered; // mapping from all rows to filtered rows

    void prepareNoCommentFilter();
};

#endif // GCODETABLEMODEL_H
