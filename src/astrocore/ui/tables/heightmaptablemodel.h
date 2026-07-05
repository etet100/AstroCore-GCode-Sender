// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef HEIGHTMAPTABLEMODEL_H
#define HEIGHTMAPTABLEMODEL_H

#include <QObject>
#include <QAbstractTableModel>
#include <QColor>
#include <QBrush>
#include "core/heightmap/heightmap.h"

class HeightmapTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum HeightmapRole {
        PositionMmRole = Qt::UserRole + 1,
    };

    explicit HeightmapTableModel(Heightmap* heightmap, QObject *parent = nullptr);
    void setHeightmap(Heightmap* heightmap);

    // Notify views that heightmap was cleared (call after Heightmap::reset())
    void clear();
    // Notify views that heightmap was resized (call after Heightmap is reconfigured)
    void resize(int cols, int rows);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

signals:
    void dataChangedByUserInput();

private slots:
    void onHeightmapChanged();

private:
    Heightmap* m_heightmap = nullptr;

    QColor cellColor(double value) const;
};

#endif // HEIGHTMAPTABLEMODEL_H
