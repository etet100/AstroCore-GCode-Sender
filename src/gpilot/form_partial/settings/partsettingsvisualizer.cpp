// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsvisualizer.h"
#include "ui_partsettingsvisualizer.h"

partSettingsVisualizer::partSettingsVisualizer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsVisualizer)
{
    ui->setupUi(this);
}

partSettingsVisualizer::~partSettingsVisualizer()
{
    delete ui;
}
