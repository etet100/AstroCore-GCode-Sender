// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "gcodeitemdelegate.h"
#include "ui/utils/thememanager.h"
#include "utils/utils.h"
#include <QApplication>
#include <QTableView>
#include <QPainter>

GCodeItemDelegate::GCodeItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
    m_dark = ThemeManager::instance().dark();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool dark) {
        m_dark = dark;
    });
}

void GCodeItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    // Background
    if (opt.state & QStyle::State_Selected) {
        painter->fillRect(opt.rect, opt.palette.highlight());
    } else {
        GCodeItem::States state = (GCodeItem::States)index.data(Qt::UserRole + 2).toInt();
        QColor backgroundColor = m_dark ? m_stateColorsDark[state] : m_stateColorsLight[state];
        bool isCurrent = index.data(Qt::UserRole + 3).toBool();
        if (isCurrent) {
            backgroundColor = backgroundColor.darker(m_dark ? 150 : 120);
        }
        painter->fillRect(opt.rect, backgroundColor);
    }

    QString mainText = index.data(Qt::DisplayRole).toString();
    QString comment = index.data(Qt::UserRole + 1).toString();

    QFont font = opt.font;
    painter->setFont(font);

    QColor textColor = (opt.state & QStyle::State_Selected)
                           ? opt.palette.highlightedText().color()
                           : opt.palette.text().color();
    QRect r = opt.rect.adjusted(5, 0, -5, 0);

    painter->setPen(textColor);
    painter->drawText(r, index.data(Qt::TextAlignmentRole).toInt(), mainText);

    if (comment.length()) {
        textColor = (opt.state & QStyle::State_Selected)
            ? m_dark ? opt.palette.highlightedText().color().darker(130) : opt.palette.highlightedText().color().lighter(130)
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
