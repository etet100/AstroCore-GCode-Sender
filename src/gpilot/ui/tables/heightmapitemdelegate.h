// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef HEIGHTMAPITEMDELEGATE_H
#define HEIGHTMAPITEMDELEGATE_H

#include <QStyledItemDelegate>

class HeightmapItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit HeightmapItemDelegate(double min = -5.0, double max = 5.0, QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override;
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

private:
    double m_min;
    double m_max;
};

#endif // HEIGHTMAPITEMDELEGATE_H
