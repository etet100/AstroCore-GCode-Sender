// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef GCODEITEMDELEGATE_H
#define GCODEITEMDELEGATE_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include "core/gcode/gcode.h"

class GCodeItemDelegate : public QStyledItemDelegate
{
    public:
        using QStyledItemDelegate::QStyledItemDelegate;
        void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
        GCodeItemDelegate(QObject *parent = nullptr);

    private:
        inline static QMap<GCodeItem::States, QColor> m_stateColorsLight = {
            {GCodeItem::States::InQueue, QColor("#eef5ff")},
            {GCodeItem::States::Sent, QColor("#fff4e5")},
            {GCodeItem::States::Processed, QColor("#e6ffed")},
            {GCodeItem::States::Error, QColor("#ffe6e6")},
            {GCodeItem::States::Skipped, QColor("#f0f0f0")},
            {GCodeItem::States::Comment, QColor("#f9f9f9")},
            {GCodeItem::States::Aborted, QColor("#ffe6e6")}
        };

        inline static QMap<GCodeItem::States, QColor> m_stateColorsDark = {
            {GCodeItem::States::InQueue, QColor("#2a3b4e")},
            {GCodeItem::States::Sent, QColor("#4e3b2a")},
            {GCodeItem::States::Processed, QColor("#2a4e36")},
            {GCodeItem::States::Error, QColor("#4e2a2a")},
            {GCodeItem::States::Skipped, QColor("#3a3a3a")},
            {GCodeItem::States::Comment, QColor("#3e3e3e")},
            {GCodeItem::States::Aborted, QColor("#4e2a2a")}
        };

        bool m_dark;
};

#endif // GCODEITEMDELEGATE_H
