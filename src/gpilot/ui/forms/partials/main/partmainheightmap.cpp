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

    connect(ui->txtAreaHeight, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaWidth, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaX, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaY, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaX1, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaX2, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaY1, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);
    connect(ui->txtAreaY2, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::onAreaChanged);

    ui->fraAreaX1X2->hide();
    connect(ui->chkAreaWidthHeight, &QCheckBox::toggled, this, [this](bool checked){
        ui->fraAreaWH->setVisible(checked);
        ui->fraAreaX1X2->setVisible(!checked);
    });
}

PartMainHeightmap::~PartMainHeightmap()
{
    delete ui;
}

QRectF PartMainHeightmap::areaRectFromTextboxes()
{
    QRectF rect;

    if (ui->chkAreaWidthHeight->isChecked()) {
        rect.setX(ui->txtAreaX->value());
        rect.setY(ui->txtAreaY->value());
        rect.setWidth(ui->txtAreaWidth->value());
        rect.setHeight(ui->txtAreaHeight->value());
    } else {
        rect.setX(ui->txtAreaX1->value());
        rect.setY(ui->txtAreaY1->value());
        rect.setRight(ui->txtAreaX2->value());
        rect.setBottom(ui->txtAreaY2->value());
    }

    return rect;
}

void PartMainHeightmap::applyHeightmapConfiguration(ConfigurationHeightmap &configurationHeightmap)
{
    ui->txtAreaX->setValue(configurationHeightmap.areaX1());
    ui->txtAreaX1->setValue(configurationHeightmap.areaX1());
    ui->txtAreaY->setValue(configurationHeightmap.areaY1());
    ui->txtAreaY1->setValue(configurationHeightmap.areaY1());
    ui->txtAreaWidth->setValue(configurationHeightmap.areaX2() - configurationHeightmap.areaX1());
    ui->txtAreaHeight->setValue(configurationHeightmap.areaY2() - configurationHeightmap.areaY1());
    ui->txtAreaX2->setValue(configurationHeightmap.areaY2());
    ui->txtAreaY2->setValue(configurationHeightmap.areaY2());
    ui->chkShowArea->setChecked(configurationHeightmap.areaShow());

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

void PartMainHeightmap::on_cmdAreaFromGCode_clicked()
{
    // Request extremes from heightmap, setHeightmapBorderRect will be called
    // in response
    emit extremesRequired();
}

void PartMainHeightmap::setHeightmapBorderRect(QRectF rect)
{
    if (!qIsNaN(rect.width()) && !qIsNaN(rect.height())) {
        // WH
        ui->txtAreaX->setValue(rect.x());
        ui->txtAreaY->setValue(rect.y());
        ui->txtAreaWidth->setValue(rect.width());
        ui->txtAreaHeight->setValue(rect.height());
        // X1X2
        ui->txtAreaX1->setValue(rect.x());
        ui->txtAreaY1->setValue(rect.y());
        ui->txtAreaX2->setValue(rect.x() + rect.width());
        ui->txtAreaY2->setValue(rect.y() + rect.height());
    }
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

void PartMainHeightmap::emitShowVisualizationChanged()
{
    emit showVisualizationChanged(
        {
            ui->chkShowArea->isChecked(),
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

void PartMainHeightmap::onAreaChanged()
{
    if (ui->chkAreaWidthHeight->isChecked()) {
        ui->txtAreaX1->setValue(ui->txtAreaX->value());
        ui->txtAreaY1->setValue(ui->txtAreaY->value());
        ui->txtAreaX2->setValue(ui->txtAreaX->value() + ui->txtAreaWidth->value());
        ui->txtAreaY2->setValue(ui->txtAreaY->value() + ui->txtAreaHeight->value());
    } else {
        ui->txtAreaX->setValue(ui->txtAreaX1->value());
        ui->txtAreaY->setValue(ui->txtAreaY1->value());
        ui->txtAreaWidth->setValue(ui->txtAreaX2->value() -  ui->txtAreaX1->value());
        ui->txtAreaHeight->setValue(ui->txtAreaY2->value() -  ui->txtAreaY1->value());
    }

    emit areaChanged(areaRectFromTextboxes());
}

void PartMainHeightmap::onGridParametersChanged()
{
    emit gridParametersChanged(
        QPoint(ui->txtGridX->value(), ui->txtGridY->value()),
        { ui->txtGridZBottom->value(),  ui->txtGridZTop->value() },
        ui->txtProbeFeed->value(),
        QPoint(ui->txtInterpolationStepX->value(), ui->txtInterpolationStepY->value())
    );
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

void PartMainHeightmap::on_chkShowArea_toggled(bool checked)
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
