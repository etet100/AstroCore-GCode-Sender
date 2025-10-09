// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "partmainspindle.h"
#include "ui_partmainspindle.h"

partMainSpindle::partMainSpindle(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainSpindle)
{
    ui->setupUi(this);
}

partMainSpindle::~partMainSpindle()
{
    delete ui;
}
