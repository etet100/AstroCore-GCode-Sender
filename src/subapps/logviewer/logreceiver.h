#ifndef LOGRECEIVER_H
#define LOGRECEIVER_H

#include <QObject>
#include <QLocalSocket>

class FrmLog;

// Receives log entries from the main application over a QLocalSocket.
// Each message is framed as: quint32 blockSize | quint8 type | QString msg
class LogReceiver : public QObject
{
        Q_OBJECT

    public:
        explicit LogReceiver(FrmLog* logForm, QObject* parent = nullptr);
        void setSocket(QLocalSocket* socket);

    private slots:
        void onReadyRead();
        void onDisconnected();

    private:
        FrmLog* m_logForm;
        QLocalSocket* m_socket = nullptr;
        quint32 m_blockSize = 0;
};

#endif // LOGRECEIVER_H
