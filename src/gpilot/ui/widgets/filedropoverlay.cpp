#include "filedropoverlay.h"
#include "ui_filedropoverlay.h"

FileDropOverlay::FileDropOverlay(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::FileDropOverlay)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_AlwaysStackOnTop);
}

FileDropOverlay::~FileDropOverlay()
{
    delete ui;
}

void FileDropOverlay::showForbidden()
{
    // ui->label->setText("Invalid File");
    // ui->label->setStyleSheet("color: red");
    ui->label->setPixmap(QPixmap(":/images/filedrop_invalid.png"));
    m_isValid = false;
}

void FileDropOverlay::showValid()
{
    // ui->label->setText("Drop Here");
    // ui->label->setStyleSheet("color: green");
    ui->label->setPixmap(QPixmap(":/images/filedrop_valid.png"));
    m_isValid = true;
}

bool FileDropOverlay::valid()
{
    return m_isValid;
}
