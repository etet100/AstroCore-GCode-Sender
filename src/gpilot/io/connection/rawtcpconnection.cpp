#include "rawtcpconnection.h"
#include <QThread>

RawTcpConnection::RawTcpConnection(QObject *parent) : Connection(parent)
{
}

RawTcpConnection::~RawTcpConnection()
{
    close();
}

bool RawTcpConnection::open()
{
    qDebug() << "[IP][RawTCP] Connecting to " << m_host << ":" << m_port;

    if (m_state == ConnectionState::Connecting) {
        return false;
    }
    if (m_state == ConnectionState::Connected) {
        return true;
    }

    setState(ConnectionState::Connecting);

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::readyRead, this, &RawTcpConnection::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &RawTcpConnection::close);
    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        qDebug() << "[IP][RawTCP] Connected to " << m_host << ":" << m_port;
        setState(ConnectionState::Connected);
    });
    m_socket->connectToHost(m_host, quint16(m_port));

    return true;
}

void RawTcpConnection::setHost(QString host)
{    
    m_host = host;
}

void RawTcpConnection::setPort(int port)
{
    m_port = port;
}

void RawTcpConnection::sendByteArray(QByteArray byteArray)
{
    #ifdef DEBUG_RAW_TCP_COMMUNICATION
        qDebug() << "[IP][RawTCP] >> " << byteArray;
    #endif

    m_socket->write(byteArray.data(), 1);
    m_socket->flush();

    flushOutgoingData();
}

void RawTcpConnection::sendLine(QString line)
{
    #ifdef DEBUG_RAW_TCP_COMMUNICATION
        qDebug() << "[IP][RawTCP] >> " << line;
    #endif

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    flushOutgoingData();
}

void RawTcpConnection::close()
{
    if (m_socket == nullptr || m_state == ConnectionState::Disconnecting) {
        return;
    }

    qDebug() << "[IP][RawTCP] Closing connection";
    setState(ConnectionState::Disconnecting);

    QTcpSocket* oldSocket = m_socket;
    m_socket = nullptr;

    oldSocket->close();
    oldSocket->deleteLater();

    setState(ConnectionState::Disconnected);
}

void RawTcpConnection::flushOutgoingData()
{
    assert(m_socket != nullptr);

    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
    }
}

void RawTcpConnection::onReadyRead()
{
    while (m_socket->bytesAvailable() > 0) {
        m_incoming += m_socket->readAll();
        processIncomingData();
    }
}

void RawTcpConnection::processIncomingData()
{
    while (true) {
        if (m_incoming.isEmpty()) {
            return;
        }
        int pos = m_incoming.indexOf("\n");
        if (pos == -1) {
            return;
        }

        QString line = m_incoming.left(pos).trimmed();
        m_incoming.remove(0, pos + 1);

        #ifdef DEBUG_RAW_TCP_COMMUNICATION
            qDebug() << "[IP][RawTCP] << " << line;
        #endif

        emit this->lineReceived(line);
    }
}
