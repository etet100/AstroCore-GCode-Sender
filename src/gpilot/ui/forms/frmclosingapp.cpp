#include "frmclosingapp.h"
#include "ui_frmclosingapp.h"

FrmClosingApp::FrmClosingApp(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::FrmClosingApp)
{
    ui->setupUi(this);
}

FrmClosingApp::~FrmClosingApp()
{
    delete ui;
}
