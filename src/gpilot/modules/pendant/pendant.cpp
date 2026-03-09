// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "core/globals.h"
#include "pendant.h"
#include "core/config/configuration.h"
#include <QTcpSocket>
#include "circularbuffer.h"
#include <QTimer>
#include <CRC.h>

#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif

enum class CommPacketType: uint8_t {
    STATE = 0,
    WIFI_CONFIG = 1,
    PING = 2,
    STEP_SIZE_CONFIG = 3,
    FEED_RATE_CONFIG = 4,
    CMD = 5,
    MAX,
};

enum class CommunicationMode: uint8_t {
    NONE = 0,
    SERIAL_,
    WIFI,
};

enum class CmdType: uint8_t {
    START = 0,
    STOP,
    PAUSE,
    HOME,
    RESET,
    SPINDLE,
};

#define COMM_PACKET_VERSION 1
#define COMM_PREAMBLE 0xAA55
#define COMM_START 0xAA55
#define COMM_START_BYTE_1 0x55
#define COMM_START_BYTE_2 0xAA
#define COMM_HEAD_START_1_POS 0
#define COMM_HEAD_START_2_POS 1
#define COMM_HEAD_VERSION_POS 2
#define COMM_HEAD_SIZE_POS 3
#define COMM_HEAD_TYPE_POS 4

PACK(struct CommHeader
{
    uint16_t start = COMM_PREAMBLE;
    uint8_t version = COMM_PACKET_VERSION;
    uint8_t size;
    uint8_t type;
});

PACK(struct CommFooter
{
    uint8_t crc;
});

PACK(struct StateMessage
{
    CommHeader header;
    float x;
    float y;
    float z;
    uint8_t machineState;
    CommunicationMode mode;
    char selectedAxis;
    CommFooter footer;
});

PACK(struct WifiConfigMessage
{
    CommHeader header;
    char ssid[20];
    char password[20];
    char clientIp[16];
    CommFooter footer;
});

#define CHOICES_COUNT 12

PACK(struct StepSizeConfigMessage
{
    CommHeader header;
    float selections[CHOICES_COUNT];
    bool separateZ;
    CommFooter footer;
});

PACK(struct PingMessage
     {
         CommHeader header;
         float selections[CHOICES_COUNT];
         bool separateZ;
         CommFooter footer;
     });

PACK(struct FeedRateConfigMessage
{
    CommHeader header;
    float selections[CHOICES_COUNT];
    bool separateZ;
    CommFooter footer;
});

PACK(struct CmdMessage
{
    CommHeader header;
    CmdType cmd;
    CommFooter footer;
});

template <typename T>
T vmax(T a) {
    return a;
}

template <typename T, typename... Args>
T vmax(T a, Args... args) {
    T b = vmax(args...);

    return (a > b) ? a : b;
}

#define COMM_MAX_PACKET_SIZE vmax(sizeof(StateMessage), \
        sizeof(WifiConfigMessage), sizeof(PingMessage), \
        sizeof(StepSizeConfigMessage), sizeof(FeedRateConfigMessage), sizeof(CmdMessage) \
)

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
            static CircularBuffer<256> buffer;

            static uint8_t packetType = 255;
            static uint8_t packetSize;

            if (buffer.free() < (size_t) m_socket->bytesAvailable()) {
                qDebug() << "[Pendant] Buffer overflow";
            } else {
                QByteArray data = m_socket->readAll();
                buffer.push((uint8_t*)data.data(), data.size());
            }

            while (buffer.size() >= sizeof(CommHeader)) {
                if (packetType == 255) {
                    while (buffer.size() >= sizeof(CommHeader)) {
                        if (
                            buffer[COMM_HEAD_START_1_POS] != COMM_START_BYTE_1 || buffer[COMM_HEAD_START_2_POS] != COMM_START_BYTE_2 ||
                            buffer[COMM_HEAD_VERSION_POS] != COMM_PACKET_VERSION ||
                            buffer[COMM_HEAD_TYPE_POS] >= (uint8_t)CommPacketType::MAX || buffer[COMM_HEAD_SIZE_POS] > COMM_MAX_PACKET_SIZE
                            ) {
                            buffer.skip(1);
                            continue;
                        }

                        packetType = buffer[COMM_HEAD_TYPE_POS];
                        packetSize = buffer[COMM_HEAD_SIZE_POS];
                        break;
                    }
                }

                if (packetType < 255 && buffer.size() >= packetSize) {
                    switch (packetType) {
                        // case (uint8_t)CommPacketType::STATE: {
                        //     StateMessage msg;
                        //     buffer.get((uint8_t*)&msg, packetSize);
                        //     //Serial.printf("CRC: %d %d %d\n", msg.footer.crc, calcCRC8((uint8_t*)&msg, packetSize - sizeof(Footer)), packetSize);
                        //     if (msg.footer.crc != calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter))) {
                        //         Serial.printf("State CRC: %d != %d, %d", msg.footer.crc, calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter)), packetSize);
                        //         break;
                        //     }
                        //     //Serial.printf("State: %6.2f %6.2f %6.2f %d\n", msg.x, msg.y, msg.z, (int)msg.mode);

                        //     state.setPos(Axis::X, msg.x);
                        //     state.setPos(Axis::Y, msg.y);
                        //     state.setPos(Axis::Z, msg.z);
                        //     state.setMachineState(msg.machineState);
                        //     state.triggerUpdatedEvent();

                        //     this->updateLastMessageTime();
                        //     break;
                        // }
                        // case (uint8_t)CommPacketType::WIFI_CONFIG: {
                        //     WifiConfigMessage msg;
                        //     buffer.get((uint8_t*)&msg, packetSize);
                        //     //Serial.printf("CRC: %d %d %d\n", msg.footer.crc, calcCRC8((uint8_t*)&msg, packetSize - sizeof(Footer)), packetSize);
                        //     if (msg.footer.crc != calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter))) {
                        //         Serial.println("Wifi cfg CRC error");
                        //         break;
                        //     }
                        //     //Serial.printf("Wifi config: %s %s %s\n", msg.ssid, msg.password, msg.clientIp);

                        //     this->updateLastMessageTime();
                        //     break;
                        // }
                        // case (uint8_t)CommPacketType::STEP_SIZE_CONFIG: {
                        //     StepSizeConfigMessage msg;
                        //     buffer.get((uint8_t*)&msg, packetSize);
                        //     if (msg.footer.crc != calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter))) {
                        //         qDebug() << "[Pendant] Step size CRC error";
                        //         break;
                        //     }
                        //     this->updateLastMessageTime();
                        //     break;
                        // }
                        // case (uint8_t)CommPacketType::FEED_RATE_CONFIG: {
                        //     FeedRateConfigMessage msg;
                        //     buffer.get((uint8_t*)&msg, packetSize);
                        //     if (msg.footer.crc != calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter))) {
                        //         qDebug() << "[Pendant] Feed rate CRC error";
                        //         break;
                        //     }
                        //     this->updateLastMessageTime();
                        //     break;
                        // }
                        case (uint8_t)CommPacketType::PING: {
                            PingMessage msg;
                            buffer.get((uint8_t*)&msg, packetSize);
                            if (msg.footer.crc != calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter))) {
                                qDebug() << "[Pendant] Ping CRC error";
                                break;
                            }
                            this->updateLastMessageTime();
                            break;
                        }
                        case (uint8_t)CommPacketType::CMD: {
                            CmdMessage msg;
                            buffer.get((uint8_t*)&msg, packetSize);
                            if (msg.footer.crc != calcCRC8((uint8_t*)&msg, packetSize - sizeof(CommFooter))) {
                                qDebug() << "[Pendant] Cmd CRC error";
                                break;
                            }
                            qDebug() << "[Pendant] Received command:" << (int)msg.cmd;
                            // Command received (echo or confirmation from server)
                            this->updateLastMessageTime();
                            break;
                        }
                        default: {
                            // Unknown packet type - skip it to avoid infinite loop
                            qDebug() << "[Pendant] Unknown packet type:" << packetType;
                            buffer.skip(packetSize);
                            break;
                        }
                    }

                    packetType = 255;
                    continue;
                }
            }
            // m_socket->write(data);
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
    message.header.type = static_cast<uint8_t>(CommPacketType::STATE);

    QVector3D pos = m_communicator.machinePos();
    message.x = pos.x();
    message.y = pos.y();
    message.z = pos.z();
    message.machineState = (uint8_t) m_communicator.machineState();

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(StateMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(StateMessage));
}

void Pendant::sendWifiConfig()
{
    qDebug() << "[Pendant] Sending wifi config";

    WifiConfigMessage message;

    message.header.size = sizeof(WifiConfigMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::WIFI_CONFIG);

    strcpy(message.ssid, "ssid");
    strcpy(message.password, "password");

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(WifiConfigMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(WifiConfigMessage));
}

void Pendant::sendStepSizeSelections()
{
    // qDebug() << "[Pendant] Sending step size config";

    StepSizeConfigMessage message;

    message.header.start = 0xAA55;
    message.header.size = sizeof(StepSizeConfigMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::STEP_SIZE_CONFIG);

    message.separateZ = false;

    QStringList choices = m_configuration.joggingModule().stepChoices();
    for (int i = 0; i < CHOICES_COUNT; i++) {
        message.selections[i] = i < choices.size() ? choices[i].toFloat() : 0.0f;
    }

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(StepSizeConfigMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(StepSizeConfigMessage));
}

void Pendant::sendFeedRateSelections()
{
    // qDebug() << "[Pendant] Sending feed rate config";

    FeedRateConfigMessage message;

    message.header.size = sizeof(FeedRateConfigMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::FEED_RATE_CONFIG);

    message.separateZ = m_configuration.joggingModule().separateFeedZ();

    QStringList choices = m_configuration.joggingModule().feedChoices();
    for (int i = 0; i < CHOICES_COUNT; i++) {
        message.selections[i] = i < choices.size() ? choices[i].toFloat() : 0.0f;
    }

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(FeedRateConfigMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(FeedRateConfigMessage));
}

void Pendant::updateLastMessageTime()
{

}
