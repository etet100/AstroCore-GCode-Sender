// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "gcodeitemdelegate.h"
#include "gcodetablemodel.h"
#include "core/gcode/gcode.h"

GCodeItemDelegate::GCodeItemDelegate() {}

#include <QApplication>
#include <QTableView>
#include <QPainter>

void GCodeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Background
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    } else {
        index.data(Qt::UserRole + 2).toInt();
        switch (index.data(Qt::UserRole + 2).toInt()) {
            case GCodeItem::States::InQueue:
                painter->fillRect(opt.rect, QColor("#eef5ff"));
                break;
            case GCodeItem::States::Sent:
                painter->fillRect(opt.rect, QColor("#fff4e5"));
                break;
            case GCodeItem::States::Processed:
                painter->fillRect(opt.rect, QColor("#e6ffed"));
                break;
            case GCodeItem::States::Skipped:
                painter->fillRect(opt.rect, QColor("#f0f0f0"));
                break;
            case GCodeItem::States::Comment:
                painter->fillRect(opt.rect, QColor("#f9f9f9"));
                break;
        }
    }

    QString mainText = index.data(Qt::DisplayRole).toString();
    QString comment = index.data(Qt::UserRole + 1).toString();

    QFont font = opt.font;
    painter->setFont(font);

    QColor textColor = (opt.state & QStyle::State_Selected)
                           ? opt.palette.highlightedText().color()
                           : QColor(Qt::black);
    QRect r = opt.rect.adjusted(5, 0, -5, 0);

    painter->setPen(textColor);
    painter->drawText(r, index.data(Qt::TextAlignmentRole).toInt(), mainText);

    if (comment.length()) {
        textColor = (opt.state & QStyle::State_Selected)
            ? opt.palette.highlightedText().color().lighter(130)
            : QColor(Qt::gray);

        // Calculate width of main text to position comment correctly
        QFontMetrics fm(font);
        int mainWidth = fm.horizontalAdvance(mainText);

        QRect commentRect = r.adjusted(mainWidth + 5, 0, 0, 0);

        painter->setPen(textColor);
        painter->drawText(commentRect, Qt::AlignVCenter | Qt::AlignLeft, comment);
    }

    painter->restore();
}
