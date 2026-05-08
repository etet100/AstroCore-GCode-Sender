// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partmainheightmap.h"
#include "ui_partmainheightmap.h"
#include "core/heightmap/configurationheightmap.h"
#include <QPushButton>

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

    ui->cboInterpolationMode->setCurrentIndex(Heightmap::Bicubic);
    connect(ui->cboInterpolationMode, &QComboBox::currentIndexChanged, this, [this](int index){
        emit interpolationModeChanged(static_cast<Heightmap::InterpolationMode>(index));
    });

    connect(ui->chkShowArea, &QCheckBox::toggled, this, &PartMainHeightmap::emitShowVisualizationChanged);
    connect(ui->chkShowProbeGrid, &QCheckBox::toggled, this, &PartMainHeightmap::emitShowVisualizationChanged);
    connect(ui->chkShowInterpolation, &QCheckBox::toggled, this, &PartMainHeightmap::emitShowVisualizationChanged);
    connect(ui->chkUseHeightmap, &QCheckBox::toggled, this, &PartMainHeightmap::useHeightmapToggled);
    connect(ui->cmdHeightMapMode, &QPushButton::toggled, this, &PartMainHeightmap::heightmapModeToggled);
    connect(ui->cmdAreaFromGCode, &QPushButton::clicked, this, &PartMainHeightmap::extremesRequired);
    connect(ui->cmdNew, &QPushButton::clicked, this, &PartMainHeightmap::newClicked);
    connect(ui->cmdOpen, &QPushButton::clicked, this, &PartMainHeightmap::openClicked);
    connect(ui->cmdSave, &QPushButton::clicked, this, &PartMainHeightmap::saveClicked);

    connect(ui->txtGridX, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::emitGridParametersChanged);
    connect(ui->txtGridY, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::emitGridParametersChanged);
    connect(ui->txtGridZBottom, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::emitGridParametersChanged);
    connect(ui->txtGridZTop, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::emitGridParametersChanged);
    connect(ui->txtInterpolationStepX, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::emitGridParametersChanged);
    connect(ui->txtInterpolationStepY, &QDoubleSpinBox::valueChanged, this, &PartMainHeightmap::emitGridParametersChanged);
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
    QSignalBlocker blocker(this);

    ui->txtAreaX->setValue(configurationHeightmap.areaX1());
    ui->txtAreaX1->setValue(configurationHeightmap.areaX1());
    ui->txtAreaY->setValue(configurationHeightmap.areaY1());
    ui->txtAreaY1->setValue(configurationHeightmap.areaY1());
    ui->txtAreaWidth->setValue(configurationHeightmap.areaX2() - configurationHeightmap.areaX1());
    ui->txtAreaHeight->setValue(configurationHeightmap.areaY2() - configurationHeightmap.areaY1());
    ui->txtAreaX2->setValue(configurationHeightmap.areaY2());
    ui->txtAreaY2->setValue(configurationHeightmap.areaY2());

    ui->txtGridX->setValue(configurationHeightmap.gridX());
    ui->txtGridY->setValue(configurationHeightmap.gridY());
    ui->txtGridZTop->setValue(configurationHeightmap.gridZTop());
    ui->txtGridZBottom->setValue(configurationHeightmap.gridZBottom());
    ui->txtProbeFeed->setValue(configurationHeightmap.probeFeed());

    ui->txtInterpolationStepX->setValue(configurationHeightmap.interpolationStepX());
    ui->txtInterpolationStepY->setValue(configurationHeightmap.interpolationStepY());
    ui->cboInterpolationMode->setCurrentIndex(configurationHeightmap.interpolationType());
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

void PartMainHeightmap::resetOpenFile()
{
    ui->txtHeightMapName->setText("");
}

void PartMainHeightmap::resetUseHeighmap()
{
    ui->chkUseHeightmap->setChecked(false);
}

void PartMainHeightmap::updateControlsState(const UiState& state)
{
    ui->cmdHeightMapMode->setEnabled(state.files.heightmapOpened);
    ui->chkUseHeightmap->setEnabled(!heightmapMode() && state.files.heightmapOpened);
}

void PartMainHeightmap::setOpenFile(QString filePath)
{
    ui->txtHeightMapName->setText(filePath);
}

void PartMainHeightmap::setHeightmapAreaRect(QRectF rect)
{
    if (qIsNaN(rect.width()) || qIsNaN(rect.height())) {
        return;
    }

    {
        QSignalBlocker blocker(this);

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

    emit areaChanged(rect);
}

void PartMainHeightmap::setHeightmap(Heightmap* heightmap)
{
    m_heightmap = heightmap;

    QSignalBlocker blocker(this);

    ui->txtGridX->setValue(heightmap->gridWidth());
    ui->txtGridY->setValue(heightmap->gridHeight());
    ui->txtGridZBottom->setValue(heightmap->zBottomTop().bottom);
    ui->txtGridZTop->setValue(heightmap->zBottomTop().top);
    ui->txtProbeFeed->setValue(ConfigurationHeightmap::instance().probeFeed());
    ui->txtInterpolationStepX->setValue(heightmap->interpolationStepSize().width());
    ui->txtInterpolationStepY->setValue(heightmap->interpolationStepSize().height());
    ui->cboInterpolationMode->setCurrentIndex(static_cast<int>(heightmap->interpolationMode()));
}

void PartMainHeightmap::resizeEvent(QResizeEvent *event)
{
    setMinimumHeight(sizeHint().height());

    QWidget::resizeEvent(event);
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
        QSize(ui->txtGridX->value(), ui->txtGridY->value()),
        { ui->txtGridZBottom->value(), ui->txtGridZTop->value() },
        ui->txtProbeFeed->value(),
        QSize(ui->txtInterpolationStepX->value(), ui->txtInterpolationStepY->value())
    );
}

// void PartMainHeightmap::newClicked()
// {
//     ui->cmdHeightMapMode->setChecked(true);

//     emit newClicked();
// }

void PartMainHeightmap::onAreaChanged()
{
    {
        QSignalBlocker blocker(this);

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
    }

    emit areaChanged(areaRectFromTextboxes());
}

void PartMainHeightmap::onGridParametersChanged()
{
    emit gridParametersChanged(
        QSize(ui->txtGridX->value(), ui->txtGridY->value()),
        { ui->txtGridZBottom->value(),  ui->txtGridZTop->value() },
        ui->txtProbeFeed->value(),
        QSize(ui->txtInterpolationStepX->value(), ui->txtInterpolationStepY->value())
    );
}

