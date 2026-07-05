#include "dlgeditheightmappoint.h"
#include "ui_dlgeditheightmappoint.h"

DlgEditHeightmapPoint::DlgEditHeightmapPoint(QPoint point, double height, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgEditHeightmapPoint)
    , m_point(point)
    , m_orgHeight(height)
{
    ui->setupUi(this);

    setWindowTitle(QString("Height at %1, %2").arg(point.x()).arg(point.y()));
    ui->spinHeight->setValue(height);
}

DlgEditHeightmapPoint::~DlgEditHeightmapPoint()
{
    delete ui;
}

double DlgEditHeightmapPoint::height() const
{
    return ui->spinHeight->value();
}

void DlgEditHeightmapPoint::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->spinHeight->setFocus();
}

void DlgEditHeightmapPoint::dialogButtonClick(QAbstractButton *button)
{
    if (ui->buttons->buttonRole(button) == QDialogButtonBox::ResetRole) {
        ui->spinHeight->setValue(m_orgHeight);
        ui->spinHeight->setFocus();
    }
}
