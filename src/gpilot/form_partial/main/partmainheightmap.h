// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINHEIGHTMAP_H
#define PARTMAINHEIGHTMAP_H

#include <QWidget>

namespace Ui {
class partMainHeightmap;
}

class partMainHeightmap : public QWidget
{
        Q_OBJECT

    public:
        explicit partMainHeightmap(QWidget *parent = nullptr);
        ~partMainHeightmap();

    private:
        Ui::partMainHeightmap *ui;
};

#endif // PARTMAINHEIGHTMAP_H
