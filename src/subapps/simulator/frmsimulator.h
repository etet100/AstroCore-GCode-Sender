// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef FRMSIMULATOR_H
#define FRMSIMULATOR_H

#include <QMainWindow>
#include "simulatordefs.h"

class FrmSimulator : public QMainWindow
{
    Q_OBJECT

public:
    explicit FrmSimulator(Simulator::Type type, QWidget* parent = nullptr);
};

#endif // FRMSIMULATOR_H
