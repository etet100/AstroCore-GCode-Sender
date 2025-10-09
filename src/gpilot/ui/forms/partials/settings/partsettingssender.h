// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSSENDER_H
#define PARTSETTINGSSENDER_H

#include <QWidget>

namespace Ui {
class partSettingsSender;
}

class partSettingsSender : public QWidget
{
        Q_OBJECT

    public:
        explicit partSettingsSender(QWidget *parent = nullptr);
        ~partSettingsSender();
        void setUsePauseCommands(bool use);
        void setBeforePauseCommands(const QString &commands);
        void setAfterPauseCommands(const QString &commands);
        void setUseStartCommands(bool use);
        void setUseEndCommands(bool use);
        void setStartCommands(const QString &commands);
        void setEndCommands(const QString &commands);
        void setUseToolChangeCommands(bool use);
        void setToolChangeCommands(const QString &commands);
        void setConfirmToolChangeCommandsExecution(bool confirm);
        void setPauseOnToolChange(bool pause);
        void setIgnoreResponseErrors(bool ignore);
        void setSetParseStateBeforeSendFromLine(bool set);
        bool usePauseCommands() const;
        QString beforePauseCommands() const;
        QString afterPauseCommands() const;
        bool useStartCommands() const;
        bool useEndCommands() const;
        QString startCommands() const;
        QString endCommands() const;
        bool useToolChangeCommands() const;
        QString toolChangeCommands() const;
        bool confirmToolChangeCommandsExecution() const;
        bool pauseOnToolChange() const;
        bool ignoreResponseErrors() const;
        bool setParseStateBeforeSendFromLine() const;

    private:
        Ui::partSettingsSender *ui;
};

#endif // PARTSETTINGSSENDER_H

