// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingscolors.h"
#include "ui_partsettingscolors.h"

partSettingsColors::partSettingsColors(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsColors)
{
    ui->setupUi(this);
}

partSettingsColors::~partSettingsColors()
{
    delete ui;
}

void partSettingsColors::setToolpathHighlightColor(const QColor &color)
{
    ui->clpToolpathHighlightColor->setColor(color);
}

void partSettingsColors::setToolpathZMovementColor(const QColor &color)
{
    ui->clpToolpathZMovementColor->setColor(color);
}

void partSettingsColors::setToolpathStartColor(const QColor &color)
{
    ui->clpToolpathStartColor->setColor(color);
}

void partSettingsColors::setToolpathEndColor(const QColor &color)
{
    ui->clpToolpathEndColor->setColor(color);
}

void partSettingsColors::setToolpathNormalColor(const QColor &color)
{
    ui->clpToolpathNormalColor->setColor(color);
}

void partSettingsColors::setToolpathDrawnColor(const QColor &color)
{
    ui->clpToolpathDrawnColor->setColor(color);
}

void partSettingsColors::setToolpathRapidMovementColor(const QColor &color)
{
    ui->clpToolpathRapidMovementColor->setColor(color);
}

void partSettingsColors::setVisualizerBackgroundColor(const QColor &color)
{
    ui->clpVisualizerBackgroundColor->setColor(color);
}

void partSettingsColors::setVisualizerTextColor(const QColor &color)
{
    ui->clpVisualizerTextColor->setColor(color);
}

void partSettingsColors::setVisualizerToolColor(const QColor &color)
{
    ui->clpVisualizerToolColor->setColor(color);
}

void partSettingsColors::setVisualizerTableGridColor(const QColor &color)
{
    ui->clpVisualizerTableGridColor->setColor(color);
}

QColor partSettingsColors::toolpathHighlightColor() const
{
    return ui->clpToolpathHighlightColor->color();
}

QColor partSettingsColors::toolpathZMovementColor() const
{
    return ui->clpToolpathZMovementColor->color();
}

QColor partSettingsColors::toolpathStartColor() const
{
    return ui->clpToolpathStartColor->color();
}

QColor partSettingsColors::toolpathEndColor() const
{
    return ui->clpToolpathEndColor->color();
}

QColor partSettingsColors::toolpathNormalColor() const
{
    return ui->clpToolpathNormalColor->color();
}

QColor partSettingsColors::toolpathDrawnColor() const
{
    return ui->clpToolpathDrawnColor->color();
}

QColor partSettingsColors::toolpathRapidMovementColor() const
{
    return ui->clpToolpathRapidMovementColor->color();
}

QColor partSettingsColors::visualizerBackgroundColor() const
{
    return ui->clpVisualizerBackgroundColor->color();
}

QColor partSettingsColors::visualizerTextColor() const
{
    return ui->clpVisualizerTextColor->color();
}

QColor partSettingsColors::visualizerToolColor() const
{
    return ui->clpVisualizerToolColor->color();
}

QColor partSettingsColors::visualizerTableGridColor() const
{
    return ui->clpVisualizerTableGridColor->color();
}
