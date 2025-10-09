// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSVISUALIZER_H
#define PARTSETTINGSVISUALIZER_H

#include <QWidget>

namespace Ui {
class partSettingsVisualizer;
}

class partSettingsVisualizer : public QWidget
{
        Q_OBJECT

    public:
        explicit partSettingsVisualizer(QWidget *parent = nullptr);
        ~partSettingsVisualizer();

    private:
        Ui::partSettingsVisualizer *ui;
};

#endif // PARTSETTINGSVISUALIZER_H
