#include "openaimanager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

OpenAIManager::OpenAIManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

OpenAIManager::~OpenAIManager()
{
}

void OpenAIManager::setApiKey(const QString &key)
{
    m_apiKey = key;
}

bool OpenAIManager::sendRequest(const QString &prompt, const QString &model)
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not set");

        return false;
    }

    if (prompt.isEmpty()) {
        emit errorOccurred("Prompt cannot be empty");

        return false;
    }

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;

    QJsonArray messages;
    messages.append(message);

    QJsonObject json;
    json["model"] = model;
    json["messages"] = messages;
    json["temperature"] = 0.7;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = m_networkManager->post(request, data);
    connect(reply, &QNetworkReply::finished, this, &OpenAIManager::onReplyFinished);

    return true;
}

bool OpenAIManager::listModels()
{
    if (m_apiKey.isEmpty()) {
        emit errorOccurred("API key not set");

        return false;
    }

    QUrl url("https://api.openai.com/v1/models");
    QNetworkRequest request(url);

    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this]() {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
        if (!reply) {
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            return;
        }

        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (!doc.isObject()) {
            return;
        }

        QJsonObject obj = doc.object();
        if (!obj.contains("data")) {
            return;
        }
        QJsonArray data = obj["data"].toArray();

        QStringList models;
        for (auto i : data) {
            QJsonObject modelObj = i.toObject();
            QString modelId = modelObj["id"].toString();
            models.append(modelId);
        }

        qDebug() << "Available models:" << models;

        emit modelsListed(models);
    });

    return true;
}

void OpenAIManager::onReplyFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        return;
    }

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QString parsedResponse = parseResponse(response);

        if (!parsedResponse.isEmpty()) {
            emit responseReceived(parsedResponse);
        } else {
            emit errorOccurred("Failed to parse response");
        }
    } else {
        emit errorOccurred(QString("Network error: %1").arg(reply->errorString()));
    }

    reply->deleteLater();
}

QString OpenAIManager::parseResponse(const QByteArray &data)
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        return QString();
    }

    QJsonObject obj = doc.object();

    if (obj.contains("error")) {
        QJsonObject error = obj["error"].toObject();
        emit errorOccurred(QString("API error: %1").arg(error["message"].toString()));
        return QString();
    }

    if (!obj.contains("choices")) {
        return QString();
    }

    QJsonArray choices = obj["choices"].toArray();
    if (choices.isEmpty()) {
        return QString();
    }

    QJsonObject firstChoice = choices[0].toObject();
    QJsonObject message = firstChoice["message"].toObject();

    return message["content"].toString();
}
