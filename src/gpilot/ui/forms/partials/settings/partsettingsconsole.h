// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSCONSOLE_H
#define PARTSETTINGSCONSOLE_H

#include <QWidget>

namespace Ui {
class partSettingsConsole;
}

class partSettingsConsole : public QWidget
{
        Q_OBJECT

    public:
        explicit partSettingsConsole(QWidget *parent = nullptr);
        ~partSettingsConsole();
        void setShowProgramCommands(bool value);
        bool showProgramCommands() const;
        void setShowUICommands(bool value);
        bool showUICommands() const;
        void setShowSystemCommands(bool value);
        bool showSystemCommands() const;
        void setCommandAutoCompletion(bool value);
        bool commandAutoCompletion() const;
        void setDarkBackgroundMode(bool value);
        bool darkBackgroundMode() const;

    private:
        Ui::partSettingsConsole *ui;
};

#endif // PARTSETTINGSCONSOLE_H
