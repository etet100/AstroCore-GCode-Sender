#include "partsettingsai.h"
#include "ui_partsettingsai.h"

PartSettingsAI::PartSettingsAI(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsAI)
{
    ui->setupUi(this);
}

PartSettingsAI::~PartSettingsAI()
{
    delete ui;
}

void PartSettingsAI::setOpenAIKey(const QString &key)
{
    ui->txtOpenAIKey->setText(key);
}

QString PartSettingsAI::openAIKey() const
{
    return ui->txtOpenAIKey->text();
}
