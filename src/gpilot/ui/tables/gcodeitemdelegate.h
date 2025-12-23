// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef GCODEITEMDELEGATE_H
#define GCODEITEMDELEGATE_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>

class GCodeItemDelegate : public QStyledItemDelegate
{
    public:
        using QStyledItemDelegate::QStyledItemDelegate;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        GCodeItemDelegate();
};

#endif // GCODEITEMDELEGATE_H
