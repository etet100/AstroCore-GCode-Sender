// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSVISUALIZER_H
#define PARTSETTINGSVISUALIZER_H

#include <QWidget>

namespace Ui {
class partSettingsVisualizer;
}

class PartSettingsVisualizer : public QWidget
{
        Q_OBJECT

    public:
        explicit PartSettingsVisualizer(QWidget *parent = nullptr);
        ~PartSettingsVisualizer();

    private:
        Ui::partSettingsVisualizer *ui;
};

#endif // PARTSETTINGSVISUALIZER_H
