// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "qpushbuttonwithmenu.h"
#include <QMenu>
#include <QDebug>
#include <QAbstractButton>
#include <QStylePainter>
#include <QStyleOptionButton>

QPushButtonWithMenu::QPushButtonWithMenu(QWidget *parent) : QPushButton(parent)
{
    m_menu = new QMenu(this);
    setMenu(m_menu);
}

QMenu *QPushButtonWithMenu::menu() const
{
    return m_menu;
}

void QPushButtonWithMenu::mousePressEvent(QMouseEvent *e)
{
    if (!isOnArrow(e->pos())) {
        emit clicked();

        return;
    }

    QPushButton::mousePressEvent(e);
}

bool QPushButtonWithMenu::isOnArrow(const QPoint &pos) const
{
    QStyleOptionButton option;
    initStyleOption(&option);

    int menuButtonWidth = style()->pixelMetric(QStyle::PM_MenuButtonIndicator, &option, this) * 1.6;

    return pos.x() >= width() - menuButtonWidth;
}
