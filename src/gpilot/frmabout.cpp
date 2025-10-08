// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QDesktopServices>
#include "frmabout.h"
#include "ui_frmabout.h"
#include <QFile>

frmAbout::frmAbout(QWidget *parent) :
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

frmAbout::~frmAbout()
{
    delete ui;
}

void frmAbout::onCmdOkClicked()
{
    this->hide();
}

void frmAbout::onLblAboutLinkActivated(const QString &link)
{
    QDesktopServices::openUrl(link);
}
