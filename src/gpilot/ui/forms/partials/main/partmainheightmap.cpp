// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partmainheightmap.h"
#include "ui_partmainheightmap.h"

partMainHeightmap::partMainHeightmap(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainHeightmap)
{
    ui->setupUi(this);
}

partMainHeightmap::~partMainHeightmap()
{
    delete ui;
}
