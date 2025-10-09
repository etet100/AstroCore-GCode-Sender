// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef QPUSHBUTTONWITHMENU_H
#define QPUSHBUTTONWITHMENU_H

#include <QPushButton>
#include <QMouseEvent>

class QPushButtonWithMenu : public QPushButton
{
    public:
        QPushButtonWithMenu(QWidget *parent = nullptr);
        QMenu *menu() const;

    protected:
        void mousePressEvent(QMouseEvent *e) override;
        void mouseReleaseEvent(QMouseEvent *e) override;
        void paintEvent(QPaintEvent *) override;

    private:
        bool isOnArrow(const QPoint &pos) const;
        QMenu *m_menu;

};

#endif // QPUSHBUTTONWITHMENU_H
