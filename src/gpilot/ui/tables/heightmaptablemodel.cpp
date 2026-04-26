// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "heightmaptablemodel.h"
#include "ui/utils/thememanager.h"
#include <QColor>
#include <QtMath>

HeightmapTableModel::HeightmapTableModel(Heightmap* heightmap, QObject* parent)
    : QAbstractTableModel(parent)
{
    setHeightmap(heightmap);
}

void HeightmapTableModel::setHeightmap(Heightmap* heightmap)
{
    if (m_heightmap == heightmap) {
        return;
    }

    beginResetModel();
    if (m_heightmap) {
        disconnect(m_heightmap, &Heightmap::changed, this, &HeightmapTableModel::onHeightmapChanged);
    }
    m_heightmap = heightmap;
    if (m_heightmap) {
        connect(m_heightmap, &Heightmap::changed, this, &HeightmapTableModel::onHeightmapChanged);
    }
    endResetModel();
}

void HeightmapTableModel::onHeightmapChanged()
{
    beginResetModel();
    endResetModel();
}

void HeightmapTableModel::clear()
{
    beginResetModel();
    endResetModel();
}

void HeightmapTableModel::resize(int cols, int rows)
{
    Q_UNUSED(cols)
    Q_UNUSED(rows)
    beginResetModel();
    endResetModel();
}

int HeightmapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_heightmap ? m_heightmap->gridHeight() : 0;
}

int HeightmapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return m_heightmap ? m_heightmap->gridWidth() : 0;
}

QVariant HeightmapTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || !m_heightmap) {
        return QVariant();
    }

    int rows = m_heightmap->gridHeight();
    if (index.row() >= rows || index.column() >= m_heightmap->gridWidth()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        // Flip rows so row 0 shows the bottom of the heightmap (highest Y)
        double val = m_heightmap->at(index.column(), (rows - 1) - index.row());

        return qIsNaN(val) ? QString() : QString::number(val, 'f', 3);
    }

    if (role == Qt::UserRole) {
        return m_heightmap->at(index.column(), index.row());
    }

    if (role == Qt::BackgroundRole) {
        double val = m_heightmap->at(index.column(), (rows - 1) - index.row());
        QColor c = cellColor(val);
        if (c.isValid()) {
            return QBrush(c);
        }

        return QVariant();
    }

    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role == PositionMmRole) {
        QPointF startPos = m_heightmap->startPos();
        QSizeF stepSize = m_heightmap->stepSize();
        double mmX = startPos.x() + index.column() * stepSize.width();
        double mmY = startPos.y() + ((rows - 1) - index.row()) * stepSize.height();

        return QPointF(mmX, mmY);
    }

    return QVariant();
}

QColor HeightmapTableModel::cellColor(double value) const
{
    if (qIsNaN(value) || !m_heightmap) {
        return QColor();
    }

    auto minMax = m_heightmap->valuesMinMax();
    if (qIsNaN(minMax.min) || qIsNaN(minMax.max)) {
        return QColor();
    }

    double range = minMax.max - minMax.min;
    double hue = qFuzzyIsNull(range) ? 0.33 : 0.67 * (minMax.max - value) / range;
    double v = ThemeManager::instance().dark() ? 0.45 : 1.0;

    return QColor::fromHsvF(qBound(0.0, hue, 0.67), 0.55, v);
}

bool HeightmapTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || !m_heightmap) {
        return false;
    }

    if (role == Qt::EditRole) {
        bool ok = false;
        double v = value.toDouble(&ok);
        if (!ok || v < -5.0 || v > 5.0) {
            return false;
        }
        int rows = m_heightmap->gridHeight();
        // m_heightmap->at(index.column(), (rows - 1) - index.row()) = v;
        m_heightmap->setHeightAt(QPoint(index.column(), (rows - 1) - index.row()), v);
        emit dataChangedByUserInput();
    } else if (role == Qt::UserRole) {
        // m_heightmap->at(index.column(), index.row()) = value.toDouble();
        m_heightmap->setHeightAt(QPoint(index.column(), index.row()), value.toDouble());
    } else {
        return false;
    }

    emit dataChanged(index, index, {role});

    return true;
}

QVariant HeightmapTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    return QString::number(section + 1);
}

Qt::ItemFlags HeightmapTableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
}
