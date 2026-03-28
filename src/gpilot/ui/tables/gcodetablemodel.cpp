// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodetablemodel.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"

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
    if (m_data == nullptr || !index.isValid() || index.row() >= rowCount()) {
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
    if (role == Qt::DisplayRole) {
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return item.lineNumber;
            case GCodeTableColumn::Command: return item.command;
            case GCodeTableColumn::State:
                switch (item.state) {
                    case GCodeItem::InQueue: return tr("In queue");
                    case GCodeItem::Sent: return tr("Sent");
                    case GCodeItem::Processed: return tr("Processed");
                    case GCodeItem::Error: return tr("Error");
                    case GCodeItem::Skipped: return tr("Skipped");
                    case GCodeItem::Comment: return tr("Comment");
                    case GCodeItem::Aborted: return tr("Aborted");
                }
                return tr("Unknown");
            case GCodeTableColumn::Response: return item.response;
        }
    } else if (role == Qt::EditRole) {
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Command: return item.line;
        }
    }

    if (role == Qt::UserRole + 1 && (GCodeTableColumn)index.column() == GCodeTableColumn::Command) {
        return item.comment;
    }

    if (role == Qt::UserRole + 2) {
        return item.state;
    }

    if (role == Qt::UserRole + 3) {
        return (m_data->commandIndex() == rowNumber); // is current command
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
        // Inserting new line at the end
        if (row == m_data->count()) {
            const GCodeItem newItem = GcodePreprocessorUtils::parseLine(value.toString());
            *m_data << newItem;

            int rowCount = m_filtered ? m_filteredRows.size() : (m_data->count() + 1);
            beginInsertRows(QModelIndex(), rowCount - 1, rowCount - 1);
            endInsertRows();

            qDebug() << "Appending new line:" << newItem.command << "with comment:" << newItem.comment;

            return false;
        }

        GCodeItem& item = m_data->at(row);
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return false;
            case GCodeTableColumn::Command: {
                const GCodeItem newItem = GcodePreprocessorUtils::parseLine(value.toString());
                item.command = newItem.command;
                item.comment = newItem.comment;
                break;
            }
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
    applyFilters();
    // connect(m_data, &GCode::linesUpdated, this, &GCodeTableModel::notifyLinesUpdated, Qt::UniqueConnection);
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

void GCodeTableModel::updateLines(int from, int to)
{
    notifyLinesUpdated(from, to);
}

int GCodeTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (m_data == nullptr) {
        return m_filtered ? 0 : 1;
    }

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
    m_showComments = visible;
    beginResetModel();
    applyFilters();
    endResetModel();
}

void GCodeTableModel::showComments()
{
    setCommentsVisible(true);
}

void GCodeTableModel::hideComments()
{
    setCommentsVisible(false);
}

void GCodeTableModel::setFilter(const QString &text)
{
    m_filterText = text.trimmed();
    beginResetModel();
    applyFilters();
    endResetModel();
}

void GCodeTableModel::clearFilter()
{
    setFilter(QString());
}

int GCodeTableModel::toFilteredIndex(int index) const
{
    if (m_filtered) {
        return m_allRowsToFiltered[index];
    } else {
        return index;
    }
}

void GCodeTableModel::applyFilters()
{
    m_filtered = !m_showComments || !m_filterText.isEmpty();

    m_filteredRows.clear();
    m_allRowsToFiltered.clear();

    if (!m_filtered) {
        return;
    }

    int i = 0;
    int k = 0;
    for (auto& row : *m_data) {
        bool visible = true;
        if (!m_showComments && row.group == GCodeItemGroup::Comment) {
            visible = false;
        } else if (!m_filterText.isEmpty()) {
            if (!row.command.contains(m_filterText, Qt::CaseInsensitive) && (!m_showComments || !row.comment.contains(m_filterText, Qt::CaseInsensitive))) {
                visible = false;
            }
        }
        if (visible) {
            k = m_filteredRows.size();
            m_filteredRows.append(i);
        }
        i++;
        m_allRowsToFiltered.append(k);
    }

    assert(m_data->count() == m_allRowsToFiltered.count());
}
