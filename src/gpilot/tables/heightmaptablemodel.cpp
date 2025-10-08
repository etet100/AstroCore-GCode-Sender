// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmaptablemodel.h"

HeightmapTableModel::HeightmapTableModel(QObject *parent) : QAbstractTableModel(parent)
{
    m_data.append(QVector<double>());
}

void HeightmapTableModel::resize(int cols, int rows)
{
    foreach (QVector<double> row, m_data) row.clear();

    m_data.clear();

    for (int i = 0; i < rows; i++) {
        QVector<double> row;
        for (int j = 0; j < cols; j++) {
            row.append(qQNaN());
        }
        m_data.append(row);
    }
}

QVariant HeightmapTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (index.row() >= m_data.count() || index.column() >= m_data[0].count()) return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        return QString::number(m_data[(m_data.count() - 1) - index.row()][index.column()], 'f', 3);
    }

    if (role == Qt::UserRole) {
        return m_data[index.row()][index.column()];
    }

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    return QVariant();
}

bool HeightmapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    m_data[role == Qt::EditRole ? (m_data.count() - 1) - index.row() : index.row()][index.column()] = value.toDouble();

    if (role == Qt::EditRole) emit dataChangedByUserInput();

    return true;
}

bool HeightmapTableModel::insertRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    m_data.insert(row, QVector<double>());
    return true;
}

bool HeightmapTableModel::removeRow(int row, const QModelIndex &parent)
{
    Q_UNUSED(parent)

    m_data.remove(row);
    return true;
}

void HeightmapTableModel::clear()
{
    m_data.clear();
}

int HeightmapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_data.count();
}

int HeightmapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_data[0].count();
}

QVariant HeightmapTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if (role != Qt::DisplayRole) return QVariant();
    return QString::number(section + 1);
}

Qt::ItemFlags HeightmapTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}

