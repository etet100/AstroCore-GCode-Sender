// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSSHORTCUTS_H
#define PARTSETTINGSSHORTCUTS_H

#include <QWidget>

namespace Ui {
class frmSettingsShortcuts;
}

class ShortcutNode;

class PartSettingsShortcuts : public QWidget
{
    Q_OBJECT

public:
    explicit PartSettingsShortcuts(QWidget *parent = nullptr);
    ~PartSettingsShortcuts();

    // Fill the table from ShortcutsManager. Call this after actions are registered.
    void populate();

    // Write edited shortcuts from the table back to their QActions.
    void applyChanges();

    // Reset all shortcuts in the table to built-in defaults.
    void setDefaults();

private:
    Ui::frmSettingsShortcuts *ui;

    void addNodeRows(const ShortcutNode *node, int depth);
};

#endif // PARTSETTINGSSHORTCUTS_H
