// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "core/globals.h"
#include "pendant.h"
#include "defines.h"
#include "core/config/configuration.h"
#include <QTcpSocket>
#include "circularbuffer.h"
#include <QTimer>
#include <QDateTime>
#include <CRC.h>

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
            static qint64 packetTypeSetTime = 0;

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
                        packetTypeSetTime = QDateTime::currentMSecsSinceEpoch();
                        break;
                    }
                }

                if (packetType < 255) {
                    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
                    if ((currentTime - packetTypeSetTime) > 50) {
                        qDebug() << "[Pendant] Packet timeout, resetting";
                        packetType = 255;
                        buffer.skip(1);
                        continue;
                    }
                }

                if (packetType < 255 && buffer.size() >= packetSize) {
                    uint8_t packetData[COMM_MAX_PACKET_SIZE];
                    buffer.get(packetData, packetSize);

                    uint8_t receivedCrc = packetData[packetSize - 1];
                    uint8_t calculatedCrc = calcCRC8(packetData, packetSize - 1);
                    if (receivedCrc != calculatedCrc) {
                        qDebug() << "[Pendant] CRC error for packet type:" << packetType;
                        packetType = 255;
                        continue;
                    }

                    switch (packetType) {
                        case (uint8_t)CommPacketType::PING:
                            handlePingMessage(packetData, packetSize);
                            break;
                        case (uint8_t)CommPacketType::CMD:
                            handleCmdMessage(packetData, packetSize);
                            break;
                        case (uint8_t)CommPacketType::STEP_SIZE_CHANGED:
                            handleStepSizeChangedMessage(packetData, packetSize);
                            break;
                        case (uint8_t)CommPacketType::FEED_RATE_CHANGED:
                            handleFeedRateChangedMessage(packetData, packetSize);
                            break;
                        case (uint8_t)CommPacketType::JOG:
                            handleJogMessage(packetData, packetSize);
                            break;
                        default:
                            qDebug() << "[Pendant] Unknown packet type:" << packetType;
                            break;
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
        sendWifiConfig("", "");
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

void Pendant::sendWifiConfig(const QString &ssid, const QString &password)
{
    qDebug() << "[Pendant] Sending wifi config";

    if (ssid.length() >= COMM_SSID_PASSWORD_MAX_LEN || password.length() >= COMM_SSID_PASSWORD_MAX_LEN) {
        qWarning() << "[Pendant] SSID or password too long to send";
        return;
    }

    WifiConfigMessage message;

    message.header.size = sizeof(WifiConfigMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::WIFI_CONFIG);

    strncpy(message.ssid, ssid.toUtf8().constData(), sizeof(message.ssid) - 1);
    message.ssid[sizeof(message.ssid) - 1] = '\0';

    strncpy(message.password, password.toUtf8().constData(), sizeof(message.password) - 1);
    message.password[sizeof(message.password) - 1] = '\0';

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(WifiConfigMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(WifiConfigMessage));
}

void Pendant::sendStepSizeSelections()
{
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
    m_lastMessageTime = QDateTime::currentMSecsSinceEpoch();
}

bool Pendant::isConnectionTimedOut(qint64 timeoutMs) const
{
    if (m_lastMessageTime == 0) {
        return false; // No messages received yet
    }

    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    return (currentTime - m_lastMessageTime) > timeoutMs;
}

void Pendant::handlePingMessage(const uint8_t* data, uint8_t size)
{
    PingMessage msg;
    memcpy(&msg, data, size);

    this->updateLastMessageTime();
}

void Pendant::handleCmdMessage(const uint8_t* data, uint8_t size)
{
    CmdMessage msg;
    memcpy(&msg, data, size);

    qDebug() << "[Pendant] Received command:" << (int)msg.cmd;
    this->updateLastMessageTime();
}

void Pendant::handleStepSizeChangedMessage(const uint8_t* data, uint8_t size)
{
    StepSizeChangedMessage msg;
    memcpy(&msg, data, size);

    qDebug() << "[Pendant] Step size changed:" << msg.value;
    this->updateLastMessageTime();
}

void Pendant::handleFeedRateChangedMessage(const uint8_t* data, uint8_t size)
{
    FeedRateChangedMessage msg;
    memcpy(&msg, data, size);

    qDebug() << "[Pendant] Feed rate changed:" << msg.value;
    this->updateLastMessageTime();
}

void Pendant::handleJogMessage(const uint8_t* data, uint8_t size)
{
    JogMessage msg;
    memcpy(&msg, data, size);

    qDebug() << "[Pendant] Jog - X:" << (int)msg.x << "Y:" << (int)msg.y << "Z:" << (int)msg.z;
    this->updateLastMessageTime();
}
