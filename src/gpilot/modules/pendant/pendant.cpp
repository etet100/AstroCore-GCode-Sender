// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "pendant.h"

#include <QTcpSocket>
//#include <CircularBuffer.hpp>
#include <QTimer>
#include <CRC.h>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

enum class Axis {
    None = -1,
    X = 0,
    Y,
    Z,
};

enum class PacketType: uint8_t {
    STATE = 0,
    WIFI_CONFIG = 1,
    PING = 2,
    STEP_SIZE_CONFIG = 3,
    FEED_RATE_CONFIG = 4,
};

enum class CommunicationMode: uint8_t {
    NONE = 0,
    SERIAL_,
    WIFI,
};

PACK(struct Header
{
    uint16_t start; // 0xAA55
    uint8_t size;
    uint8_t type;
});

PACK(struct Footer
{
    uint8_t crc;
});

PACK(struct StateMessage
{
    Header header;
    float x;
    float y;
    float z;
    uint8_t machineState;
    CommunicationMode mode;
    char selectedAxis;
    Footer footer;
});

PACK(struct WifiConfigMessage
{
    Header header;
    char ssid[20];
    char password[20];
    char clientIp[16];
    Footer footer;
});

PACK(struct StepSizeConfigMessage
{
    Header header;
    float stepSize[12];
    Footer footer;
});

PACK(struct FeedRateConfigMessage
{
    Header header;
    float feedRate[12];
    Footer footer;
});

class Queue {
    private:
    //CircularBuffer<char, 256> buffer;
};

Pendant::Pendant(QObject *parent, Communicator &communicator) : QObject{parent}, m_communicator{communicator}
{
    qDebug() << "Pendant created";

    this->m_server = new QTcpServer(this);
    this->m_server->listen(QHostAddress::Any, 5555);

    connect(this->m_server, &QTcpServer::newConnection, [this]() {
        qDebug() << "New pendant connection";

        m_socket = this->m_server->nextPendingConnection();
        connect(m_socket, &QTcpSocket::readyRead, [this]() {
            QByteArray data = m_socket->readAll();
            m_socket->write(data);
        });

        QTimer *timer = new QTimer(this);
        timer->setInterval(50);

        sendFeedRateConfig();
        sendStepSizeConfig();
        sendWifiConfig();

        connect(m_socket, &QTcpSocket::disconnected, [this, timer]() {
            timer->stop();
            timer->deleteLater();
            m_socket->deleteLater();
            m_socket = nullptr;
            qDebug() << "Pendant disconnected";
        });

        connect(timer, &QTimer::timeout, [this]() {
            sendState();
            // WifiConfigMessage wifiMessage;
            // wifiMessage.header.start = 0xAA55;
            // wifiMessage.header.size = sizeof(WifiConfigMessage);
            // wifiMessage.header.type = static_cast<uint8_t>(PacketType::WIFI_CONFIG);
            // strcpy(wifiMessage.ssid, "ssid");
            // strcpy(wifiMessage.password, "password");
            // strcpy(wifiMessage.clientIp, "127.0.0.1");
            // wifiMessage.footer.crc = calcCRC8((uint8_t*)&wifiMessage, sizeof(WifiConfigMessage) - sizeof(Footer));

            // socket->write((char*)&wifiMessage, sizeof(WifiConfigMessage));
        });
        timer->start();
    });
}

void Pendant::sendState()
{
    StateMessage message;
    message.header.start = 0xAA55;
    message.header.size = sizeof(StateMessage);
    message.header.type = static_cast<uint8_t>(PacketType::STATE);

    QVector3D pos = m_communicator.machinePos();
    message.x = pos.x();
    message.y = pos.y();
    message.z = pos.z();
    message.machineState = (uint8_t) m_communicator.deviceState();

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(StateMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(StateMessage));
}

void Pendant::sendWifiConfig()
{
    qDebug() << "Sending wifi config";

    WifiConfigMessage message;

    message.header.start = 0xAA55;
    message.header.size = sizeof(WifiConfigMessage);
    message.header.type = static_cast<uint8_t>(PacketType::WIFI_CONFIG);

    strcpy(message.ssid, "ssid");
    strcpy(message.password, "password");

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(WifiConfigMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(WifiConfigMessage));
}

void Pendant::sendStepSizeConfig()
{
    qDebug() << "Sending step size config";

    StepSizeConfigMessage message;

    message.header.start = 0xAA55;
    message.header.size = sizeof(StepSizeConfigMessage);
    message.header.type = static_cast<uint8_t>(PacketType::STEP_SIZE_CONFIG);

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(StepSizeConfigMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(StepSizeConfigMessage));
}

void Pendant::sendFeedRateConfig()
{
    qDebug() << "Sending feed rate config";

    FeedRateConfigMessage message;

    message.header.start = 0xAA55;
    message.header.size = sizeof(FeedRateConfigMessage);
    message.header.type = static_cast<uint8_t>(PacketType::FEED_RATE_CONFIG);

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(FeedRateConfigMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(FeedRateConfigMessage));
}
