// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef VALIDATORS_H
#define VALIDATORS_H

#include <QValidator>

class QCommaSeparatedValidator : public QValidator
{
    public:
        QCommaSeparatedValidator(QObject *parent = 0): QValidator(parent) {}

        QValidator::State validate(QString &input, int &pos) const
        {
            Q_UNUSED(pos)

            QStringList elements = input.split(",");
            for(QString& str : elements) {
                str = str.trimmed();
            }
            static QRegularExpression rx("^.{1,}$");
            elements = elements.filter(rx);

            for (const QString& str : elements) {
                if (!validateElement(str)) {
                    return QValidator::Invalid;
                }
            }

            return QValidator::Acceptable;
        }

    protected:
        virtual bool validateElement(const QString &element) const = 0;
};

class QCommaSeparatedIntValidator : public QCommaSeparatedValidator
{
    public:
        QCommaSeparatedIntValidator(QObject *parent = 0): QCommaSeparatedValidator(parent) {}

        bool validateElement(const QString &element) const override
        {
            static QRegularExpression rx("^[\\d]*$");

            return element.indexOf(rx) != -1;
        }
};

class QCommaSeparatedDoubleValidator : public QCommaSeparatedValidator
{
    public:
        QCommaSeparatedDoubleValidator(QObject *parent = 0): QCommaSeparatedValidator(parent) {}

        bool validateElement(const QString &element) const override
        {
            static QRegularExpression rx("^[\\d]*(\\.[\\d]{1,2})?$");

            return element.indexOf(rx) != -1;
        }
};

#endif // VALIDATORS_H
