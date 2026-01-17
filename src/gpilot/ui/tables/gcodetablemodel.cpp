// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodetablemodel.h"

GCodeTableModel::GCodeTableModel(GCode* data, QObject *parent) :
    QAbstractTableModel(parent),
    m_data(data)
{
    m_headers << tr("#") << tr("Command") << tr("State") << tr("Response");

    if (data) {
        connect(data, &GCode::linesUpdated, this, &GCodeTableModel::notifyLinesUpdated, Qt::UniqueConnection);
    }
}

void GCodeTableModel::notifyLinesUpdated(int fromLine, int toLine)
{
    emit dataChanged(
        index(toFilteredIndex(fromLine), 0),
        index(toFilteredIndex(toLine), columnCount() - 1));
}

QVariant GCodeTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    int rowNumber = index.row();
    if (m_filtered) {
        rowNumber = m_filteredRows[index.row()];
    }

    // Last, empty line for easier appending new lines
    if (rowNumber == m_data->count()) {
        // Show <new command> as grayed comment in the Command column
        return role == Qt::UserRole + 1  && (GCodeTableColumn)index.column() == GCodeTableColumn::Command ? "<new command>" : QVariant();
    }

    GCodeItem& item = m_data->at(rowNumber);
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return index.row() + 1;
            case GCodeTableColumn::Command: return item.command;
            case GCodeTableColumn::State:
                switch (item.state) {
                    case GCodeItem::InQueue: return tr("In queue");
                    case GCodeItem::Sent: return tr("Sent");
                    case GCodeItem::Processed: return tr("Processed");
                    case GCodeItem::Error: return tr("Error");
                    case GCodeItem::Skipped: return tr("Skipped");
                    case GCodeItem::Comment: return tr("Comment");
                }
                return tr("Unknown");
            case GCodeTableColumn::Response: return item.response;
        }
    }

    if (role == Qt::UserRole + 1 && (GCodeTableColumn)index.column() == GCodeTableColumn::Command) {
        return item.comment;
    }

    if (role == Qt::UserRole + 2) {
        return item.state;
    }

    if (role == Qt::TextAlignmentRole) {
        switch ((GCodeTableColumn)index.column()) {
            case GCodeTableColumn::Number: return Qt::AlignCenter;
            default: return Qt::AlignVCenter;
        }
    }

    // Without returning valid QVariant here, initStyleOption displays Invalid description
    if (role == Qt::FontRole) {
        return QVariant();
    }

    return QVariant();
}

bool GCodeTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        int row = index.row();
        if (row == m_data->count()) {
            // Append new line
            // beginInsertRows(QModelIndex(), row, row);
            // m_data->append(GCodeItem());
            // endInsertRows();

            return false;
        }

        GCodeItem& item = m_data->at(row);
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return false;
            case GCodeTableColumn::Command: item.command = value.toString(); break;
            // case 2: m_data[index.row()].state = value.toInt(); break;
            case GCodeTableColumn::Response: item.response = value.toString(); break;
        }
        emit dataChanged(index, index);

        return true;
    }

    return false;
}

void GCodeTableModel::setProgram(GCode* data)
{
    beginResetModel();
    m_data = data;
    m_filteredRows.clear();
    m_allRowsToFiltered.clear();
    m_filtered = false;
    connect(m_data, &GCode::linesUpdated, this, &GCodeTableModel::notifyLinesUpdated, Qt::UniqueConnection);
    endResetModel();
}

bool GCodeTableModel::insertRow(int row, const QModelIndex &parent)
{
    if (row > rowCount()) return false;

    beginInsertRows(parent, row, row);
    m_data->insert(row, GCodeItem());
    endInsertRows();

    return true;
}

bool GCodeTableModel::removeRow(int row, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row);
    m_data->removeAt(row);
    endRemoveRows();

    return true;
}

bool GCodeTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    m_data->erase(row, row + count);
    endRemoveRows();

    return true;
}

void GCodeTableModel::clear()
{
    beginResetModel();
    m_data->clear();
    endResetModel();
}

void GCodeTableModel::update()
{
    beginResetModel();
    endResetModel();
}

int GCodeTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    // +1 add empty row at the end for easier appending new lines
    return m_filtered ? m_filteredRows.size() : (m_data->count() + 1);
}

int GCodeTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 4;
}

QVariant GCodeTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation == Qt::Horizontal) return m_headers.at(section);
    else return QString::number(section + 1);
}

Qt::ItemFlags GCodeTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    if (index.column() == 1) return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    else return QAbstractTableModel::flags(index);
}

void GCodeTableModel::setCommentsVisible(bool visible)
{
    beginResetModel();
    if (visible) {
        m_filteredRows.clear();
        m_filtered = false;
    } else {
        prepareNoCommentFilter();
    }
    endResetModel();
}

int GCodeTableModel::toFilteredIndex(int index) const
{
    if (m_filtered) {
        return m_allRowsToFiltered[index];
    } else {
        return index;
    }
}

void GCodeTableModel::prepareNoCommentFilter()
{
    m_filteredRows.clear();
    m_allRowsToFiltered.clear();
    int i = 0;
    int k = 0;
    for (auto& row : *m_data) {
        if (row.group != GCodeItemGroup::Comment) {
            k = m_filteredRows.size();
            m_filteredRows.append(i);
        }
        i++;
        m_allRowsToFiltered.append(k);
    }

    // qDebug() << m_filteredRows;
    // qDebug() << m_allRowsToFiltered;

    assert(m_data->count() == m_allRowsToFiltered.count());

    m_filtered = true;
}
