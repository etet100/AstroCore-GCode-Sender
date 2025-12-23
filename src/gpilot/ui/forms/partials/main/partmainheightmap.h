// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINHEIGHTMAP_H
#define PARTMAINHEIGHTMAP_H

#include <QWidget>

namespace Ui {
class partMainHeightmap;
}

class PartMainHeightmap : public QWidget
{
        Q_OBJECT

    public:
        explicit PartMainHeightmap(QWidget *parent = nullptr);
        ~PartMainHeightmap();

    private:
        Ui::partMainHeightmap *ui;
};

#endif // PARTMAINHEIGHTMAP_H
