// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingscolors.h"
#include "ui_partsettingscolors.h"

PartSettingsColors::PartSettingsColors(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsColors)
{
    ui->setupUi(this);
}

PartSettingsColors::~PartSettingsColors()
{
    delete ui;
}

void PartSettingsColors::setToolpathHighlightColor(const QColor &color)
{
    ui->clpToolpathHighlightColor->setColor(color);
}

void PartSettingsColors::setToolpathZMovementColor(const QColor &color)
{
    ui->clpToolpathZMovementColor->setColor(color);
}

void PartSettingsColors::setToolpathStartColor(const QColor &color)
{
    ui->clpToolpathStartColor->setColor(color);
}

void PartSettingsColors::setToolpathEndColor(const QColor &color)
{
    ui->clpToolpathEndColor->setColor(color);
}

void PartSettingsColors::setToolpathNormalColor(const QColor &color)
{
    ui->clpToolpathNormalColor->setColor(color);
}

void PartSettingsColors::setToolpathDrawnColor(const QColor &color)
{
    ui->clpToolpathDrawnColor->setColor(color);
}

void PartSettingsColors::setToolpathRapidMovementColor(const QColor &color)
{
    ui->clpToolpathRapidMovementColor->setColor(color);
}

void PartSettingsColors::setVisualizerBackgroundColor(const QColor &color)
{
    ui->clpVisualizerBackgroundColor->setColor(color);
}

void PartSettingsColors::setVisualizerTextColor(const QColor &color)
{
    ui->clpVisualizerTextColor->setColor(color);
}

void PartSettingsColors::setVisualizerToolColor(const QColor &color)
{
    ui->clpVisualizerToolColor->setColor(color);
}

void PartSettingsColors::setVisualizerCursorColor(const QColor &color)
{
    ui->clpVisualizerCursorColor->setColor(color);
}

void PartSettingsColors::setVisualizerTableGridColor(const QColor &color)
{
    ui->clpVisualizerTableGridColor->setColor(color);
}

QColor PartSettingsColors::toolpathHighlightColor() const
{
    return ui->clpToolpathHighlightColor->color();
}

QColor PartSettingsColors::toolpathZMovementColor() const
{
    return ui->clpToolpathZMovementColor->color();
}

QColor PartSettingsColors::toolpathStartColor() const
{
    return ui->clpToolpathStartColor->color();
}

QColor PartSettingsColors::toolpathEndColor() const
{
    return ui->clpToolpathEndColor->color();
}

QColor PartSettingsColors::toolpathNormalColor() const
{
    return ui->clpToolpathNormalColor->color();
}

QColor PartSettingsColors::toolpathDrawnColor() const
{
    return ui->clpToolpathDrawnColor->color();
}

QColor PartSettingsColors::toolpathRapidMovementColor() const
{
    return ui->clpToolpathRapidMovementColor->color();
}

QColor PartSettingsColors::visualizerBackgroundColor() const
{
    return ui->clpVisualizerBackgroundColor->color();
}

QColor PartSettingsColors::visualizerTextColor() const
{
    return ui->clpVisualizerTextColor->color();
}

QColor PartSettingsColors::visualizerToolColor() const
{
    return ui->clpVisualizerToolColor->color();
}

QColor PartSettingsColors::visualizerCursorColor() const
{
    return ui->clpVisualizerCursorColor->color();
}

QColor PartSettingsColors::visualizerTableGridColor() const
{
    return ui->clpVisualizerTableGridColor->color();
}
