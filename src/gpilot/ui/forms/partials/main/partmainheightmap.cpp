// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partmainheightmap.h"
#include "ui_partmainheightmap.h"

PartMainHeightmap::PartMainHeightmap(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainHeightmap)
{
    ui->setupUi(this);
}

PartMainHeightmap::~PartMainHeightmap()
{
    delete ui;
}

QRectF PartMainHeightmap::borderRectFromTextboxes()
{
    QRectF rect;

    rect.setX(ui->txtBorderX->value());
    rect.setY(ui->txtBorderY->value());
    rect.setWidth(ui->txtBorderWidth->value());
    rect.setHeight(ui->txtBorderHeight->value());

    return rect;
}

void PartMainHeightmap::applyHeightmapConfiguration(ConfigurationHeightmap &configurationHeightmap)
{
    ui->txtBorderX->setValue(configurationHeightmap.borderX());
    ui->txtBorderY->setValue(configurationHeightmap.borderY());
    ui->txtBorderWidth->setValue(configurationHeightmap.borderWidth());
    ui->txtBorderHeight->setValue(configurationHeightmap.borderHeight());
    ui->chkShowBorder->setChecked(configurationHeightmap.borderShow());

    ui->txtGridX->setValue(configurationHeightmap.gridX());
    ui->txtGridY->setValue(configurationHeightmap.gridY());
    ui->txtGridZTop->setValue(configurationHeightmap.gridZTop());
    ui->txtGridZBottom->setValue(configurationHeightmap.gridZBottom());
    ui->txtProbeFeed->setValue(configurationHeightmap.probeFeed());
    ui->chkShowProbeGrid->setChecked(configurationHeightmap.gridShow());

    ui->txtInterpolationStepX->setValue(configurationHeightmap.interpolationStepX());
    ui->txtInterpolationStepY->setValue(configurationHeightmap.interpolationStepY());
    ui->cboInterpolationType->setCurrentIndex(configurationHeightmap.interpolationType());
    ui->chkShowInterpolation->setChecked(configurationHeightmap.interpolationShow());
}

bool PartMainHeightmap::heightmapMode()
{
    return ui->cmdHeightMapMode->isChecked();
}

bool PartMainHeightmap::useMap()
{
    return ui->chkUseHeightmap->isChecked();
}

bool PartMainHeightmap::showInterpolationGrid()
{
    return ui->chkShowInterpolation->isChecked();
}

void PartMainHeightmap::setGridUpdateEnabled()
{
    ui->txtGridX->setEnabled(true);
    ui->txtGridY->setEnabled(true);
    ui->txtGridZBottom->setEnabled(true);
    ui->txtGridZTop->setEnabled(true);
}

void PartMainHeightmap::fileClosed()
{
    ui->txtHeightMapName->setText("");
}

void PartMainHeightmap::resetUseHeighmap()
{
    ui->chkUseHeightmap->setChecked(false);
}

void PartMainHeightmap::updateControlsState(bool mainState, bool heightmapMode)
{
    //setEnabled(mainState);
    ui->cmdHeightMapMode->setEnabled(!ui->txtHeightMapName->text().isEmpty());
    ui->chkUseHeightmap->setEnabled(!heightmapMode && !ui->txtHeightMapName->text().isEmpty());
}

void PartMainHeightmap::setOpenFile(QString filePath)
{
    ui->txtHeightMapName->setText(filePath);
}

void PartMainHeightmap::on_cmdAutoBorder_clicked()
{
    // Request extremes from heightmap, setHeightmapBorderRect will be called
    // in response
    emit extremesRequired();
}

void PartMainHeightmap::setHeightmapBorderRect(QRectF rect)
{
    if (!qIsNaN(rect.width()) && !qIsNaN(rect.height())) {
        ui->txtBorderX->setValue(rect.x());
        ui->txtBorderY->setValue(rect.y());
        ui->txtBorderWidth->setValue(rect.width());
        ui->txtBorderHeight->setValue(rect.height());
    }
}

void PartMainHeightmap::on_txtBorderX_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitBorderChanged();
}

void PartMainHeightmap::on_txtBorderWidth_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitBorderChanged();
}

void PartMainHeightmap::on_txtBorderY_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitBorderChanged();
}

void PartMainHeightmap::on_txtBorderHeight_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitBorderChanged();
}

void PartMainHeightmap::on_txtGridX_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitGridParametersChanged();
}

void PartMainHeightmap::on_txtGridY_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitGridParametersChanged();
}

void PartMainHeightmap::on_txtGridZBottom_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitGridParametersChanged();
}

void PartMainHeightmap::on_txtGridZTop_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitGridParametersChanged();
}

void PartMainHeightmap::on_txtInterpolationStepX_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitGridParametersChanged();
}

void PartMainHeightmap::on_txtInterpolationStepY_valueChanged(double arg1)
{
    Q_UNUSED(arg1)

    emitGridParametersChanged();
}

void PartMainHeightmap::updateHeightmapGrid(double arg1)
{
    //TODO heightmap
    // if (sender()->property("previousValue").toDouble() != arg1 && !updateHeightmapGrid())
    //     static_cast<QDoubleSpinBox*>(sender())->setValue(sender()->property("previousValue").toDouble());
    // else sender()->setProperty("previousValue", arg1);
}

void PartMainHeightmap::emitBorderChanged()
{
    emit borderChanged(borderRectFromTextboxes());
}

void PartMainHeightmap::emitShowVisualizationChanged()
{
    emit showVisualizationChanged(
        {
            ui->chkShowBorder->isChecked(),
            ui->chkShowProbeGrid->isChecked(),
            ui->chkShowInterpolation->isChecked()
        }
    );
}

void PartMainHeightmap::emitGridParametersChanged()
{
    emit gridParametersChanged(
        QPoint(ui->txtGridX->value(), ui->txtGridY->value()),
        { ui->txtGridZBottom->value(), ui->txtGridZTop->value() },
        ui->txtProbeFeed->value(),
        QPoint(ui->txtInterpolationStepX->value(), ui->txtInterpolationStepY->value())
    );
}

void PartMainHeightmap::on_cmdNew_clicked()
{
    ui->cmdHeightMapMode->setChecked(true);

    emit newHeightmapRequested();
}

void PartMainHeightmap::on_chkShowProbeGrid_toggled(bool checked)
{
    Q_UNUSED(checked)

    emitShowVisualizationChanged();
}

void PartMainHeightmap::on_chkUseHeightmap_toggled(bool checked)
{
    emit useHeightmapToggled(checked);
}

void PartMainHeightmap::on_chkShowBorder_toggled(bool checked)
{
    Q_UNUSED(checked)

    emitShowVisualizationChanged();
}

void PartMainHeightmap::on_chkShowInterpolation_toggled(bool checked)
{
    Q_UNUSED(checked)

    emitShowVisualizationChanged();
}

void PartMainHeightmap::on_cmdHeightMapMode_toggled(bool checked)
{
    emit heightmapModeToggled(checked);
}
