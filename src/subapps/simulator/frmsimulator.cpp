// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#include "frmsimulator.h"
#include <QLabel>

FrmSimulator::FrmSimulator(Simulator::Type type, QWidget* parent)
    : QMainWindow(parent)
{
    QString typeName = Simulator::typeToName(type);

    setWindowTitle(QString("G-Pilot Simulator - %1").arg(typeName));
    resize(400, 300);

    auto* label = new QLabel(QString("Simulator: %1\nRunning...").arg(typeName), this);
    label->setAlignment(Qt::AlignCenter);
    setCentralWidget(label);
}
