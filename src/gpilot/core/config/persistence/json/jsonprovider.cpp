#include "jsonprovider.h"
#include "../../../globals.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QVariant>
#include <QDebug>

JsonProvider::JsonProvider(QObject *parent, const QString &filePath) : Provider(parent), m_filePath(filePath)
{
}

bool JsonProvider::open()
{
    QFile file(m_filePath);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray data = file.readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            if (doc.isObject()) {
                m_rootObject = doc.object();
            }
            file.close();
            return true;
        } else {
            qDebug() << "Failed to open configuration file for reading:" << m_filePath;

            return false;
        }
    } else {
        qDebug() << "Configuration file does not exist:" << m_filePath;
        m_rootObject = QJsonObject();

        return true; // Return true to allow the application to run with defaults
    }
}

void JsonProvider::close()
{
    m_rootObject = QJsonObject();
}

QJsonObject JsonProvider::getGroup(const QString& group) const
{
    if (m_rootObject.contains(group) && m_rootObject[group].isObject()) {
        return m_rootObject[group].toObject();
    }

    return QJsonObject();
}

int JsonProvider::getInt(const QString group, const QString key, int defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key) && groupObj[key].isDouble()) {
        return groupObj[key].toInt();
    }

    return defaultValue;
}

bool JsonProvider::getBool(const QString group, const QString key, bool defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key) && groupObj[key].isBool()) {
        return groupObj[key].toBool();
    }

    return defaultValue;
}

QString JsonProvider::getString(const QString group, const QString key, QString defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key) && groupObj[key].isString()) {
        return groupObj[key].toString();
    }

    return defaultValue;
}

double JsonProvider::getDouble(const QString group, const QString key, double defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key) && groupObj[key].isDouble()) {
        return groupObj[key].toDouble();
    }
    return defaultValue;
}

QVariant JsonProvider::getVariant(const QString group, const QString key, QVariant defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key)) {
        return groupObj[key].toVariant();
    }
    return defaultValue;
}

QStringList JsonProvider::getStringList(const QString group, const QString key, QStringList defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key) && groupObj[key].isArray()) {
        QJsonArray array = groupObj[key].toArray();
        QStringList result;
        for (const QJsonValue& value : array) {
            if (value.isString()) {
                result.append(value.toString());
            }
        }
        return result;
    }
    return defaultValue;
}

QVariantMap JsonProvider::getVariantMap(const QString group, const QString key, QVariantMap defaultValue)
{
    QJsonObject groupObj = getGroup(group);
    if (groupObj.contains(key) && groupObj[key].isObject()) {
        QJsonObject obj = groupObj[key].toObject();
        QVariantMap result;
        for (auto it = obj.begin(); it != obj.end(); ++it) {
            result[it.key()] = it.value().toVariant();
        }
        return result;
    }
    return defaultValue;
}
