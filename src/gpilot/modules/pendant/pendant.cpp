// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "pendant.h"
#include "core/config/configuration.h"
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

#define PACKET_VERSION 1
#define PREAMBLE 0xAA55

PACK(struct Header
{
    uint16_t start = PREAMBLE;
    uint8_t version = PACKET_VERSION;
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

#define CHOICES_COUNT 12

PACK(struct StepSizeSelectionsMessage
{
    Header header;
    float selections[CHOICES_COUNT];
    Footer footer;
    bool separateZ;
});

PACK(struct FeedRateSelectionsMessage
{
    Header header;
    float selections[CHOICES_COUNT];
    Footer footer;
    bool separateZ;
});

class Queue {
    private:
    //CircularBuffer<char, 256> buffer;
};

Pendant::Pendant(Configuration &configuration, Communicator &communicator, QObject *parent)
    : QObject{parent}
    , m_configuration{configuration}
    , m_communicator{communicator}
{
    qDebug() << "[Pendant] Created";

    m_server = new QTcpServer(this);
    m_server->listen(QHostAddress::Any, 5555);

    connect(m_server, &QTcpServer::newConnection, [this]() {
        qDebug() << "[Pendant] New pendant connection";
        if (m_socket != nullptr) {
            qWarning() << "[Pendant] Only one pendant connection is supported";
            m_server->nextPendingConnection()->disconnectFromHost();
            return;
        }

        m_socket = m_server->nextPendingConnection();
        m_server->pauseAccepting();

        connect(m_socket, &QTcpSocket::readyRead, [this]() {
            QByteArray data = m_socket->readAll();
            m_socket->write(data);
        });

        QTimer *timer = new QTimer(this);
        timer->setInterval(50);

        connect(m_socket, &QTcpSocket::disconnected, [this, timer]() {
            timer->stop();
            timer->deleteLater();
            m_socket->close();
            m_socket->deleteLater();
            m_socket = nullptr;
            m_server->resumeAccepting();
            qDebug() << "[Pendant] Pendant disconnected";
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

        sendFeedRateSelections();
        sendStepSizeSelections();
        sendWifiConfig();
    });
}

void Pendant::sendState()
{
    StateMessage message;
    message.header.size = sizeof(StateMessage);
    message.header.type = static_cast<uint8_t>(PacketType::STATE);

    QVector3D pos = m_communicator.machinePos();
    message.x = pos.x();
    message.y = pos.y();
    message.z = pos.z();
    message.machineState = (uint8_t) m_communicator.machineState();

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(StateMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(StateMessage));
}

void Pendant::sendWifiConfig()
{
    qDebug() << "[Pendant] Sending wifi config";

    WifiConfigMessage message;

    message.header.size = sizeof(WifiConfigMessage);
    message.header.type = static_cast<uint8_t>(PacketType::WIFI_CONFIG);

    strcpy(message.ssid, "ssid");
    strcpy(message.password, "password");

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(WifiConfigMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(WifiConfigMessage));
}

void Pendant::sendStepSizeSelections()
{
    qDebug() << "[Pendant] Sending step size config";

    StepSizeSelectionsMessage message;

    message.header.start = 0xAA55;
    message.header.size = sizeof(StepSizeSelectionsMessage);
    message.header.type = static_cast<uint8_t>(PacketType::STEP_SIZE_CONFIG);

    message.separateZ = false;

    QStringList choices = m_configuration.joggingModule().stepChoices();
    for (int i = 0; i < CHOICES_COUNT; i++) {
        message.selections[i] = i < choices.size() ? choices[i].toFloat() : 0.0f;
    }

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(StepSizeSelectionsMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(StepSizeSelectionsMessage));
}

void Pendant::sendFeedRateSelections()
{
    qDebug() << "[Pendant] Sending feed rate config";

    FeedRateSelectionsMessage message;

    message.header.size = sizeof(FeedRateSelectionsMessage);
    message.header.type = static_cast<uint8_t>(PacketType::FEED_RATE_CONFIG);

    message.separateZ = m_configuration.joggingModule().separateFeedZ();

    QStringList choices = m_configuration.joggingModule().feedChoices();
    for (int i = 0; i < CHOICES_COUNT; i++) {
        message.selections[i] = i < choices.size() ? choices[i].toFloat() : 0.0f;
    }

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(FeedRateSelectionsMessage) - sizeof(Footer));
    m_socket->write((char*)&message, sizeof(FeedRateSelectionsMessage));
}
