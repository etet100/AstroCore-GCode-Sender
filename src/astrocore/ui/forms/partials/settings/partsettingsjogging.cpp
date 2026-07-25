// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsjogging.h"
#include "ui_partsettingsjogging.h"
#include <QValidator>

PartSettingsJogging::PartSettingsJogging(QWidget *parent)
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
        if (state == QValidator::Invalid) {
            QPalette pal = sender->palette();
            pal.setColor(QPalette::Text, Qt::red);
            sender->setPalette(pal);
        } else {
            // Back to the inherited palette; an invalid QColor would stay applied.
            sender->setPalette(QPalette());
        }
        emit validityChanged("JoggingStepChoices", state != QValidator::Invalid);
    });

    connect(ui->txtJoggingFeedChoices, &QLineEdit::editingFinished, this, [this]() {
        QLineEdit *sender = dynamic_cast<QLineEdit*>(QObject::sender());
        QString text = sender->text();
        int pos = 0;
        auto state = m_commaSeparatedIntValidator.validate(text, pos);
        if (state == QValidator::Invalid) {
            QPalette pal = sender->palette();
            pal.setColor(QPalette::Text, Qt::red);
            sender->setPalette(pal);
        } else {
            // Back to the inherited palette; an invalid QColor would stay applied.
            sender->setPalette(QPalette());
        }
        emit validityChanged("JoggingFeedChoices", state != QValidator::Invalid);
    });
}

PartSettingsJogging::~PartSettingsJogging()
{
    delete ui;
}

void PartSettingsJogging::setStepChoices(const QStringList &choices)
{
    ui->txtJoggingStepChoices->setText(choices.join(", "));
}

QStringList PartSettingsJogging::stepChoices() const
{
    return ui->txtJoggingStepChoices->text().split(",").replaceInStrings(m_nonDigitsRegex, "");
}

void PartSettingsJogging::setFeedChoices(const QStringList &choices)
{
    ui->txtJoggingFeedChoices->setText(choices.join(", "));
}

QStringList PartSettingsJogging::feedChoices() const
{
    return ui->txtJoggingFeedChoices->text().split(",").replaceInStrings(m_nonDigitsRegex, "");
}
