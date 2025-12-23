// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_connection(this)
{
    ui->setupUi(this);

    connect(&m_connection, SIGNAL(lineReceived(QString)), this, SLOT(onConnectionLineReceived(QString)));

    m_connection.openConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onConnectionLineReceived(QString line)
{
    ui->consoleEdit->appendPlainText("<< " + line);
}

void MainWindow::sendClicked()
{
    QString command = ui->commandEdit->text().toLatin1().trimmed();
    m_connection.sendLine(command);
    ui->consoleEdit->appendPlainText(">> " + command);
    ui->commandEdit->clear();
}

void MainWindow::resetClicked()
{
    m_connection.sendByteArray(QByteArray(1, char(0x18)));
}

void MainWindow::statusClicked()
{
    m_connection.sendByteArray(QByteArray(1, '?'));
}
