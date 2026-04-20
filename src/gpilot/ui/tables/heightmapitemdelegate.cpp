// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "heightmapitemdelegate.h"
#include <QDoubleSpinBox>

HeightmapItemDelegate::HeightmapItemDelegate(double min, double max, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_min(min)
    , m_max(max)
{
}

QWidget* HeightmapItemDelegate::createEditor(QWidget* parent,
                                              const QStyleOptionViewItem&,
                                              const QModelIndex&) const
{
    auto* spin = new QDoubleSpinBox(parent);
    spin->setRange(m_min, m_max);
    spin->setDecimals(3);
    spin->setSingleStep(0.001);
    return spin;
}

void HeightmapItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* spin = qobject_cast<QDoubleSpinBox*>(editor);
    if (!spin) return;
    spin->setValue(index.data(Qt::EditRole).toDouble());
}

void HeightmapItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                          const QModelIndex& index) const
{
    auto* spin = qobject_cast<QDoubleSpinBox*>(editor);
    if (!spin) return;
    spin->interpretText();
    model->setData(index, spin->value(), Qt::EditRole);
}
