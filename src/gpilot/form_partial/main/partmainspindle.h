// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINSPINDLE_H
#define PARTMAINSPINDLE_H

#include <QWidget>

namespace Ui {
class partMainSpindle;
}

class partMainSpindle : public QWidget
{
        Q_OBJECT

    public:
        explicit partMainSpindle(QWidget *parent = nullptr);
        ~partMainSpindle();

    private:
        Ui::partMainSpindle *ui;
};

#endif // PARTMAINSPINDLE_H
