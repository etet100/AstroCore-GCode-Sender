// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSJOGGING_H
#define PARTSETTINGSJOGGING_H

#include <QWidget>
#include <QRegularExpression>
#include "utils/validators.h"

namespace Ui {
class partSettingsJogging;
}

class partSettingsJogging : public QWidget
{
        Q_OBJECT

    public:
        explicit partSettingsJogging(QWidget *parent = nullptr);
        ~partSettingsJogging();
        void setStepChoices(const QStringList &choices);
        QStringList stepChoices() const;
        void setFeedChoices(const QStringList &choices);
        QStringList feedChoices() const;

    signals:
        void validityChanged(QString widgetName, bool valid);

    private:
        Ui::partSettingsJogging *ui;
        QRegularExpression m_nonDigitsRegex;
        QCommaSeparatedIntValidator m_commaSeparatedIntValidator;
        QCommaSeparatedDoubleValidator m_commaSeparatedDoubleValidator;
};

#endif // PARTSETTINGSJOGGING_H
