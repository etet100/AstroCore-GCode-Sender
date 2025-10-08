#include "rawtcpconnection.h"
#include <QThread>

RawTcpConnection::RawTcpConnection(QObject *parent) : Connection(parent)
{
}

RawTcpConnection::~RawTcpConnection()
{

}

bool RawTcpConnection::openConnection()
{
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

void RawTcpConnection::sendByteArray(QByteArray)
{

}

void RawTcpConnection::sendLine(QString line)
{
    //flushOutgoingData();

    #ifdef DEBUG_RAW_TCP_COMMUNICATION
        qDebug() << "uCNC >> " << line;
    #endif

    std::string str = QString(line + "\n").toStdString();
    m_socket->write(str.c_str(), str.length());

    m_socket->flush();
}

void RawTcpConnection::closeConnection()
{
    m_connected = false;
    if (m_socket != nullptr) {
        m_socket->close();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
    m_server->close();
    m_server->deleteLater();
}

void RawTcpConnection::flushOutgoingData()
{
    assert(m_socket != nullptr);\

    if (m_socket->bytesToWrite()) {
        m_socket->waitForBytesWritten(5);
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
            qDebug() << "uCNC << " << line;
        #endif

        emit this->lineReceived(line);
    }
}
