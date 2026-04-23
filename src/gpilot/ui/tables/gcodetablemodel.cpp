// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodetablemodel.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"

GCodeTableModel::GCodeTableModel(GCode* program, QObject *parent) :
    QAbstractTableModel(parent),
    m_data(program)
{
    m_headers << tr("#") << tr("Command") << tr("State") << tr("Response");

    connect(&m_filter, &GCodeFilterView::aboutToReset, this, &GCodeTableModel::onFilterAboutToReset);
    connect(&m_filter, &GCodeFilterView::reset, this, &GCodeTableModel::onFilterReset);
    connect(&m_filter, &GCodeFilterView::rangeChanged, this, &GCodeTableModel::onFilterRangeChanged);

    m_filter.setSource(program);
}

void GCodeTableModel::onFilterAboutToReset()
{
    beginResetModel();
}

void GCodeTableModel::onFilterReset()
{
    endResetModel();
}

void GCodeTableModel::onFilterRangeChanged(int fromView, int toView)
{
    emit dataChanged(index(fromView, 0), index(toView, columnCount() - 1));
}

QVariant GCodeTableModel::data(const QModelIndex &index, int role) const
{
    if (m_data == nullptr || !index.isValid() || index.row() >= rowCount()) {
        return QVariant();
    }

    const int sourceRow = m_filter.isActive()
        ? m_filter.toSourceRow(index.row())
        : index.row();

    // Last, empty line for easier appending new lines (only without filters).
    if (sourceRow == m_data->count() || sourceRow < 0) {
        // Show <new command> as grayed comment in the Command column
        return role == Qt::UserRole + 1  && (GCodeTableColumn)index.column() == GCodeTableColumn::Command ? "<new command>" : QVariant();
    }

    GCodeItem& item = m_data->at(sourceRow);
    if (role == Qt::DisplayRole) {
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return item.lineNumber;
            case GCodeTableColumn::Command: return item.command();
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
            case GCodeTableColumn::Response: return m_data->response(sourceRow);
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
        return (m_data->commandIndex() == sourceRow); // is current command
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
        // Inserting new line at the end (only available when filter is inactive).
        if (!m_filter.isActive() && row == m_data->count()) {
            const GCodeItem newItem = GcodePreprocessorUtils::parseLine(value.toString());
            *m_data << newItem;

            const int newRowCount = rowCount();
            beginInsertRows(QModelIndex(), newRowCount - 1, newRowCount - 1);
            endInsertRows();

            qDebug() << "Appending new line:" << newItem.command() << "with comment:" << newItem.comment;

            return false;
        }

        const int sourceRow = m_filter.isActive() ? m_filter.toSourceRow(row) : row;
        if (sourceRow < 0) {
            return false;
        }

        GCodeItem& item = m_data->at(sourceRow);
        switch ((GCodeTableColumn)index.column())
        {
            case GCodeTableColumn::Number: return false;
            case GCodeTableColumn::Command: {
                const GCodeItem newItem = GcodePreprocessorUtils::parseLine(value.toString());
                item.line = newItem.line;
                item.comment = newItem.comment;
                item.args = newItem.args;
                break;
            }
            // case 2: m_data[index.row()].state = value.toInt(); break;
            case GCodeTableColumn::Response: m_data->setResponse(sourceRow, value.toString()); break;
        }
        emit dataChanged(index, index);

        return true;
    }

    return false;
}

void GCodeTableModel::setProgram(GCode* program)
{
    m_data = program;
    m_filter.setSource(program);
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
    onFilterRangeChanged(m_filter.toViewRow(from), m_filter.toViewRow(to));
}

int GCodeTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    if (m_data == nullptr) {
        return m_filter.isActive() ? 0 : 1;
    }

    // +1 adds the empty row at the end for appending new lines.
    // The append row is hidden when any filter is active.
    return m_filter.isActive() ? m_filter.rowCount() : (m_data->count() + 1);
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
    m_filter.setCommentsVisible(visible);
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
    m_filter.setTextFilter(text);
}

void GCodeTableModel::clearFilter()
{
    setFilter(QString());
}

int GCodeTableModel::mapToSource(int viewRow) const
{
    if (!m_filter.isActive()) {
        return viewRow;
    }
    return m_filter.toSourceRow(viewRow);
}

int GCodeTableModel::mapFromSource(int sourceRow) const
{
    if (!m_filter.isActive()) {
        return sourceRow;
    }
    return m_filter.toViewRow(sourceRow);
}
