// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#include "core/globals.h"
#include "pendant.h"
#include "defines.h"
#include "core/config/configuration.h"
#include "configurationpendant.h"
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

    connect(&ConfigurationPendant::instance(), &ConfigurationPendant::changed, this, [this]() {
        qDebug() << "[Pendant] Configuration changed";

        if (!ConfigurationPendant::instance().enabled()) {
            deinitialize();

            return;
        }

        if (m_server != nullptr && m_server->serverPort() != ConfigurationPendant::instance().port()) {
            deinitialize();
        }

        initialize();
    });

    // Configuration is loaded before this object exists, so the changed()
    // signal above never fires for the initial state — start right away.
    if (ConfigurationPendant::instance().enabled()) {
        initialize();
    }
}

void Pendant::initialize()
{
    if (m_server != nullptr) {
        return;
    }

    m_server = new QTcpServer(this);
    if (!m_server->listen(QHostAddress::Any, ConfigurationPendant::instance().port())) {
        qWarning() << "[Pendant] Cannot listen on port"
                   << ConfigurationPendant::instance().port() << ":" << m_server->errorString();
        m_server->deleteLater();
        m_server = nullptr;

        return;
    }

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
                            buffer[COMM_HEAD_TYPE_POS] >= (uint8_t)CommPacketType::MAX || buffer[COMM_HEAD_SIZE_POS] > COMM_MAX_PACKET_SIZE ||
                            // A packet shorter than header + footer would make the CRC check read out of bounds.
                            buffer[COMM_HEAD_SIZE_POS] < (uint8_t)(sizeof(CommHeader) + sizeof(CommFooter))
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
                        case (uint8_t)CommPacketType::JOGGING_PARAM:
                            handleJoggingParamMessage(packetData, packetSize);
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

void Pendant::deinitialize()
{
    QTcpSocket *socket = m_socket;
    if (socket != nullptr) {
        socket->disconnectFromHost();
        if (socket->state() != QAbstractSocket::UnconnectedState) {
            socket->waitForDisconnected();
        }
        // The disconnected handler may have already cleared the member.
        if (m_socket != nullptr) {
            m_socket->deleteLater();
            m_socket = nullptr;
        }
    }

    // Without this the old server keeps the port bound and leaks on every
    // configuration change.
    if (m_server != nullptr) {
        m_server->close();
        m_server->deleteLater();
        m_server = nullptr;
    }
}

void Pendant::sendState()
{
    if (m_socket == nullptr) {
        return;
    }

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

    if (m_socket == nullptr) {
        return;
    }

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
    if (m_socket == nullptr) {
        return;
    }

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
    if (m_socket == nullptr) {
        return;
    }

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

void Pendant::sendStepSize(float value)
{
    if (m_socket == nullptr) {
        return;
    }

    JoggingParamMessage message;
    message.header.size = sizeof(JoggingParamMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::JOGGING_PARAM);

    message.param = CommJoggingParam::STEP;
    message.value = value;

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(JoggingParamMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(JoggingParamMessage));
}

void Pendant::sendFeedRate(float value)
{
    if (m_socket == nullptr) {
        return;
    }

    JoggingParamMessage message;
    message.header.size = sizeof(JoggingParamMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::JOGGING_PARAM);

    message.param = CommJoggingParam::FEED;
    message.value = value;

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(JoggingParamMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(JoggingParamMessage));
}

void Pendant::sendFeedRateZ(float value)
{
    if (m_socket == nullptr) {
        return;
    }

    JoggingParamMessage message;
    message.header.size = sizeof(JoggingParamMessage);
    message.header.type = static_cast<uint8_t>(CommPacketType::JOGGING_PARAM);

    message.param = CommJoggingParam::FEED_Z;
    message.value = value;

    message.footer.crc = calcCRC8((uint8_t*)&message, sizeof(JoggingParamMessage) - sizeof(CommFooter));
    m_socket->write((char*)&message, sizeof(JoggingParamMessage));
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
    // size comes from the packet header — never copy more than the struct holds.
    PingMessage msg{};
    memcpy(&msg, data, qMin<size_t>(size, sizeof(msg)));

    this->updateLastMessageTime();
}

void Pendant::handleCmdMessage(const uint8_t* data, uint8_t size)
{
    // size comes from the packet header — never copy more than the struct holds.
    CmdMessage msg{};
    memcpy(&msg, data, qMin<size_t>(size, sizeof(msg)));

    qDebug() << "[Pendant] Received command:" << (int)msg.cmd;
    this->updateLastMessageTime();
}

void Pendant::handleJoggingParamMessage(const uint8_t* data, uint8_t size)
{
    // size comes from the packet header — never copy more than the struct holds.
    JoggingParamMessage msg{};
    memcpy(&msg, data, qMin<size_t>(size, sizeof(msg)));

    qDebug() << "[Pendant] Jogging param changed - Separate Z:" << (int) msg.param << msg.value;
    this->updateLastMessageTime();
}

void Pendant::handleJogMessage(const uint8_t* data, uint8_t size)
{
    // size comes from the packet header — never copy more than the struct holds.
    JogMessage msg{};
    memcpy(&msg, data, qMin<size_t>(size, sizeof(msg)));

    qDebug() << "[Pendant] Jog - X:" << (int)msg.x << "Y:" << (int)msg.y << "Z:" << (int)msg.z;
    this->updateLastMessageTime();
}
