// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSSHORTCUTS_H
#define PARTSETTINGSSHORTCUTS_H

#include <QWidget>

namespace Ui {
class frmSettingsShortcuts;
}

class partSettingsShortcuts : public QWidget
{
        Q_OBJECT

    public:
        explicit partSettingsShortcuts(QWidget *parent = nullptr);
        ~partSettingsShortcuts();
        void setDefaults();

    private:
        Ui::frmSettingsShortcuts *ui;
        void setShortcuts(QList<QAction *> acts);
};

#endif // PARTSETTINGSSHORTCUTS_H
