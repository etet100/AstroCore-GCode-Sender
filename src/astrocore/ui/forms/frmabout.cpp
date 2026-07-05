// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QDesktopServices>
#include "ui/forms/frmabout.h"
#include "ui_frmabout.h"
#include <QFile>

FrmAbout::FrmAbout(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::frmAbout)
{
    ui->setupUi(this);

    ui->lblAbout->setText(ui->lblAbout->text().arg(qApp->applicationVersion()));

    QFile file(qApp->applicationDirPath() + "/LICENSE");

    if (file.open(QIODevice::ReadOnly)) {
        ui->txtLicense->setPlainText(file.readAll());
    }
}

FrmAbout::~FrmAbout()
{
    delete ui;
}

void FrmAbout::onCmdOkClicked()
{
    this->hide();
}

void FrmAbout::onLblAboutLinkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}
