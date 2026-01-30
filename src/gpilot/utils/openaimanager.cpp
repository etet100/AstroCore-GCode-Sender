#include "openaimanager.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

OpenAIManager &OpenAIManager::instance(QString key)
{
    static OpenAIManager instance;
    if (key != "") {
        instance.setApiKey(key);
    }

    return instance;
}

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

// o.sendRequest(prompt, "gpt-4o",
//     [this](const QString &response) {
//         ui->console->append("[AI] " + response);
//     },
//     [this](const QString &error) {
//         ui->console->append("[AI][Error] " + error);
//     });

bool OpenAIManager::sendRequest(const QString &prompt, SuccessCallback onSuccess, ErrorCallback onError, const QString &model)
{
    qDebug() << "[AI] Prompt:" << prompt;

    if (m_apiKey.isEmpty()) {
        if (onError) {
            onError("API key not set");
        } else {
            emit errorOccurred("API key not set");
        }
        return false;
    }

    if (prompt.isEmpty()) {
        if (onError) {
            onError("Prompt cannot be empty");
        } else {
            emit errorOccurred("Prompt cannot be empty");
        }
        return false;
    }

    QUrl url("https://api.openai.com/v1/chat/completions");
    QNetworkRequest request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    QJsonArray messages;

    QJsonObject message;
    message["role"] = "user";
    message["content"] = prompt;
    messages.append(message);

    message["role"] = "system";
    message["content"] = "You are a CNC specialist";
    messages.append(message);

    QJsonObject json;
    json["model"] = model;
    json["messages"] = messages;
    json["temperature"] = 0.7;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    QNetworkReply *reply = m_networkManager->post(request, data);

    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QString parsedResponse = parseResponse(response);
            qDebug() << "[AI]" << parsedResponse;

            if (!parsedResponse.isEmpty()) {
                onSuccess(parsedResponse);
            } else {
                QString error = "Failed to parse response";
                onError("Failed to parse response");
            }
        } else {
            QString error = QString("Network error: %1").arg(reply->errorString());
            if (onError) {
                onError(error);
            }
            emit errorOccurred(error);
        }
        reply->deleteLater();
    });

    return true;
}

bool OpenAIManager::annotateProgram(const QString &program, SuccessCallback onSuccess, ErrorCallback onError)
{
    QStringList lines = program.split('\n');
    for (int i=0; i<lines.size(); i++) {
        lines[i] = QString("%1;%2").arg(i).arg(lines[i]);
    }

    QString prompt = QString("For each G-code line, write brief (as short as possible) description of what it does. Use lowercase."
        "Do not describe 'comment only' lines. "
        "Also describe invalid or incorrect lines. Do not return input line itself."
        "If input data contains any readable text, try to use its language for all comments."
        "Every single line return as separate json with line number to which the returned desctription refers (0-based, key=l) and description (key=d)."
        "Every input line start with line number - use the provided line numbers exactly. Do not infer or change them."
        "Do not add anything else nor wrap returned date in any tags... \n\n%1").arg(lines.join('\n'));

    return sendRequest(prompt, onSuccess, onError);
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

        qDebug() << "[AI] Available models:" << models;

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
