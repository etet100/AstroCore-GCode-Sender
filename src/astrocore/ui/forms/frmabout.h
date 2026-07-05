// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef FRMABOUT_H
#define FRMABOUT_H

#include <QDialog>

namespace Ui {
class frmAbout;
}

class FrmAbout : public QDialog
{
    Q_OBJECT

public:
    explicit FrmAbout(QWidget *parent = 0);
    ~FrmAbout();

private slots:
    void onCmdOkClicked();
    void onLblAboutLinkActivated(const QString &link);

private:
    Ui::frmAbout *ui;
};

#endif // FRMABOUT_H
