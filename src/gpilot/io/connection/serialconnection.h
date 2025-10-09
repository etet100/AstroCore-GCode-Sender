// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef SERIALCONNECTION_H
#define SERIALCONNECTION_H

#include <QtSerialPort/QSerialPort>
#include "connection.h"

class SerialConnection : public Connection
{
    Q_PROPERTY(QString portName READ portName WRITE setPortName);
    Q_PROPERTY(int baudRate READ baudRate WRITE setBaudRate);
    Q_OBJECT

    public:
        SerialConnection(QObject*);
        ~SerialConnection();
        bool openConnection() override;
        void setPortName(QString);
        void setBaudRate(int);
        void sendByteArray(QByteArray) override;
        void sendLine(QString) override;
        void closeConnection() override;
        ConfigurationConnection::ConnectionMode getSupportedMode() override { return ConfigurationConnection::ConnectionMode::SERIAL; };

    private:
        QSerialPort m_serialPort;
        void setupSerialPort();
        QString portName() { return this->m_serialPort.portName(); };
        int baudRate() { return this->m_serialPort.baudRate(); };

    private slots:
        void onSerialPortReadyRead();
        void onSerialPortError(QSerialPort::SerialPortError);
};

#endif // SERIALCONNECTION_H
