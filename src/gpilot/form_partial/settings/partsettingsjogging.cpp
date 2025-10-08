// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsjogging.h"
#include "ui_partsettingsjogging.h"
#include <QValidator>

partSettingsJogging::partSettingsJogging(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsJogging)
    , m_nonDigitsRegex("[^\\d^.]")
{
    ui->setupUi(this);

    connect(ui->txtJoggingStepChoices, &QLineEdit::editingFinished, this, [this]() {
        QLineEdit *sender = dynamic_cast<QLineEdit*>(QObject::sender());
        QString text = sender->text();
        int pos = 0;
        auto state = m_commaSeparatedDoubleValidator.validate(text, pos);
        QPalette pal = sender->palette();
        if (state == QValidator::Invalid) {
            pal.setColor(QPalette::Text, Qt::red);
        } else {
            pal.setColor(QPalette::Text, nullptr);
        }
        sender->setPalette(pal);
        emit validityChanged("JoggingStepChoices", state != QValidator::Invalid);
    });

    connect(ui->txtJoggingFeedChoices, &QLineEdit::editingFinished, this, [this]() {
        QLineEdit *sender = dynamic_cast<QLineEdit*>(QObject::sender());
        QString text = sender->text();
        int pos = 0;
        auto state = m_commaSeparatedIntValidator.validate(text, pos);
        QPalette pal = sender->palette();
        if (state == QValidator::Invalid) {
            pal.setColor(QPalette::Text, Qt::red);
        } else {
            pal.setColor(QPalette::Text, nullptr);
        }
        sender->setPalette(pal);
        emit validityChanged("JoggingFeedChoices", state != QValidator::Invalid);
    });
}

partSettingsJogging::~partSettingsJogging()
{
    delete ui;
}

void partSettingsJogging::setStepChoices(const QStringList &choices)
{
    ui->txtJoggingStepChoices->setText(choices.join(", "));
}

QStringList partSettingsJogging::stepChoices() const
{
    return ui->txtJoggingStepChoices->text().split(",").replaceInStrings(m_nonDigitsRegex, "");
}

void partSettingsJogging::setFeedChoices(const QStringList &choices)
{
    ui->txtJoggingFeedChoices->setText(choices.join(", "));
}

QStringList partSettingsJogging::feedChoices() const
{
    return ui->txtJoggingFeedChoices->text().split(",").replaceInStrings(m_nonDigitsRegex, "");
}
