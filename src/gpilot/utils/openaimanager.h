#ifndef OPENAIMANAGER_H
#define OPENAIMANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <functional>

class OpenAIManager : public QObject
{
    Q_OBJECT

public:
    using SuccessCallback = std::function<void(const QString&)>;
    using ErrorCallback = std::function<void(const QString&)>;

    static OpenAIManager& instance(QString key = "");
    ~OpenAIManager();

    void setApiKey(const QString &key);
    bool sendRequest(const QString &prompt,
                     SuccessCallback onSuccess, ErrorCallback onError,
                     const QString &model = "gpt-5-mini");
    bool annotateProgram(const QString &program, SuccessCallback onSuccess, ErrorCallback onError = nullptr);
    bool listModels();

signals:
    void responseReceived(const QString &response);
    void modelsListed(const QStringList &models);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();

private:
    QString m_apiKey;
    QNetworkAccessManager *m_networkManager;

    explicit OpenAIManager(QObject *parent = nullptr);
    QString parseResponse(const QByteArray &data);
};

#endif // OPENAIMANAGER_H
