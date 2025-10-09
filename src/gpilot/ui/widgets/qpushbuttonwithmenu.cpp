// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

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
        QPushButton::setMenu(nullptr);
    }

    QPushButton::mousePressEvent(e);
    QPushButton::setMenu(m_menu);
}

void QPushButtonWithMenu::mouseReleaseEvent(QMouseEvent *e)
{
    if (!isOnArrow(e->pos())) {
        QPushButton::setMenu(nullptr);
    }

    QPushButton::mouseReleaseEvent(e);
    QPushButton::setMenu(m_menu);
}

void QPushButtonWithMenu::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QStylePainter p(this);
    QStyleOptionButton option;
    initStyleOption(&option);
    p.drawControl(QStyle::CE_PushButton, option);

    QPen pen = p.pen();
    pen.setColor(option.palette.highlight().color());
    p.setPen(pen);

    p.drawLine(width() - 22, 3, width() - 22, height() - 4);
}

bool QPushButtonWithMenu::isOnArrow(const QPoint &pos) const
{
    return pos.x() > width() - 20;
}
