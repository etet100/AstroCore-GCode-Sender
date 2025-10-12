// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodetablemodel.h"

GCodeTableModel::GCodeTableModel(GCode &data, QObject *parent) :
    QAbstractTableModel(parent),
    m_data(data)
{
    m_headers << tr("#") << tr("Command") << tr("State") << tr("Response") << tr("Line") << tr("Args");
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
    GCodeItem item = m_data.at(rowNumber);

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
                    case GCodeItem::Skipped: return tr("Skipped");
                    case GCodeItem::Comment: return tr("Comment");
                }
                return tr("Unknown");
            case GCodeTableColumn::Response: return item.response;
            case GCodeTableColumn::Line: return item.lineNumber;
            case GCodeTableColumn::Args: return QVariant(item.args);
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

    return "";
}

bool GCodeTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return false;
            case GCodeTableColumn::Command: m_data[index.row()].command = value.toString(); break;
            // case 2: m_data[index.row()].state = value.toInt(); break;
            case GCodeTableColumn::Response: m_data[index.row()].response = value.toString(); break;
            case GCodeTableColumn::Line: m_data[index.row()].lineNumber = value.toInt(); break;
            case GCodeTableColumn::Args: m_data[index.row()].args = value.toStringList(); break;
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

bool GCodeTableModel::insertRow(int row, const QModelIndex &parent)
{
    if (row > rowCount()) return false;

    beginInsertRows(parent, row, row);
    m_data.insert(row, GCodeItem());
    endInsertRows();

    return true;
}

bool GCodeTableModel::removeRow(int row, const QModelIndex &parent)
{
    //if (!index(row, 0).isValid()) return false;

    beginRemoveRows(parent, row, row);
    m_data.removeAt(row);
    endRemoveRows();

    return true;
}

bool GCodeTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    beginRemoveRows(parent, row, row + count - 1);
    m_data.erase(m_data.begin() + row, m_data.begin() + row + count);
    endRemoveRows();

    return true;
}

void GCodeTableModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

int GCodeTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_filtered ? m_filteredRows.size() : m_data.size();
}

int GCodeTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 6;
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

void GCodeTableModel::prepareNoCommentFilter()
{
    m_filteredRows.clear();
    int i = 0;
    for (auto row : m_data) {
        if (row.group != GCodeItemGroup::Comment) {
            m_filteredRows.append(i);
        }
        i++;
    }

    m_filtered = true;
}
