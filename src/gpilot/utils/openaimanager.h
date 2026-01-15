#ifndef OPENAIMANAGER_H
#define OPENAIMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class OpenAIManager : public QObject
{
    Q_OBJECT

public:
    explicit OpenAIManager(QObject *parent = nullptr);
    ~OpenAIManager();

    void setApiKey(const QString &key);
    QString apiKey() const;

    void sendRequest(const QString &prompt, const QString &model = "gpt-3.5-turbo");

signals:
    void responseReceived(const QString &response);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();

private:
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;

    QString parseResponse(const QByteArray &data);
};

#endif // OPENAIMANAGER_H
