// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "virtualucncconnection.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
        Q_OBJECT

    public:
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow *ui;
        VirtualUCNCConnection m_connection;

    private slots:
        void onConnectionLineReceived(QString);
        void sendClicked();
        void resetClicked();
        void statusClicked();
};
#endif // MAINWINDOW_H
