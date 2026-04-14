#include "logreceiver.h"
#include "frmlog.h"
#include <QApplication>

LogReceiver::LogReceiver(FrmLog *logForm, QObject *parent)
    : QObject(parent)
    , m_logForm(logForm)
{}

void LogReceiver::setSocket(QLocalSocket *socket)
{
    m_socket = socket;
    m_blockSize = 0;
    connect(socket, &QLocalSocket::readyRead, this, &LogReceiver::onReadyRead);
    connect(socket, &QLocalSocket::disconnected, this, &LogReceiver::onDisconnected);
}

void LogReceiver::onReadyRead()
{
    QDataStream in(m_socket);
    in.setVersion(QDataStream::Qt_6_0);

    while (true) {
        if (m_blockSize == 0) {
            if (m_socket->bytesAvailable() < static_cast<qint64>(sizeof(quint32))) {
                break;
            }
            in >> m_blockSize;
        }
        if (m_socket->bytesAvailable() < static_cast<qint64>(m_blockSize)) {
            break;
        }
        quint8 type;
        QString msg;
        in >> type >> msg;
        m_blockSize = 0;
        m_logForm->log(static_cast<QtMsgType>(type), msg);
    }
}

void LogReceiver::onDisconnected()
{
    QApplication::quit();
}
