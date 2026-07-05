#include "jsonpersister.h"
#include "core/globals.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QVariant>
#include <QDir>
#include <QDebug>

JsonPersister::JsonPersister(QObject *parent, const QString &filePath) : AbstractPersister(parent), m_filePath(filePath)
{
}

bool JsonPersister::open()
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
        } else {
            qDebug() << "[Configuration][JSON] Failed to open configuration file for reading:" << m_filePath;
            return false;
        }
    } else {
        m_rootObject = QJsonObject();
    }

    return true;
}

void JsonPersister::close()
{
    if (!m_rootObject.isEmpty()) {
        saveDocument();
    }
    m_rootObject = QJsonObject();
}

bool JsonPersister::saveDocument()
{
    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_rootObject);
        file.write(doc.toJson(QJsonDocument::Indented)); // Pretty print
        file.close();

        return true;
    } else {
        qDebug() << "[Configuration][JSON] Failed to open configuration file for writing:" << m_filePath;

        return false;
    }
}

QJsonObject& JsonPersister::getOrCreateGroup(const QString& group)
{
    if (!m_rootObject.contains(group) || !m_rootObject[group].isObject()) {
        m_rootObject.insert(group, QJsonObject());
    }

    static QJsonObject temp;
    temp = m_rootObject[group].toObject();

    return temp;
}

bool JsonPersister::setInt(const QString group, const QString key, const int value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    groupObj[key] = value;
    m_rootObject[group] = groupObj;

    return true;
}

bool JsonPersister::setString(const QString group, const QString key, const QString value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    groupObj[key] = value;
    m_rootObject[group] = groupObj;
    return true;
}

bool JsonPersister::setDouble(const QString group, const QString key, const double value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    groupObj[key] = value;
    m_rootObject[group] = groupObj;

    return true;
}

bool JsonPersister::setBool(const QString group, const QString key, const bool value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    groupObj[key] = value;
    m_rootObject[group] = groupObj;
    return true;
}

bool JsonPersister::setStringList(const QString group, const QString key, const QStringList value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    QJsonArray array;
    for (const QString& item : value) {
        array.append(item);
    }
    groupObj[key] = array;
    m_rootObject[group] = groupObj;
    return true;
}

bool JsonPersister::setVariantMap(const QString group, const QString key, const QVariantMap value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    QJsonObject obj;
    QMapIterator<QString, QVariant> it(value);
    while (it.hasNext()) {
        it.next();
        obj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    groupObj[key] = obj;
    m_rootObject[group] = groupObj;
    return true;
}

bool JsonPersister::setVariantList(const QString group, const QString key, const QVariantList value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    QJsonArray array;
    for (const QVariant& item : value) {
        if (item.typeId() == QMetaType::QVariantMap) {
            QJsonObject obj;
            QMapIterator<QString, QVariant> it(item.toMap());
            while (it.hasNext()) {
                it.next();
                obj[it.key()] = QJsonValue::fromVariant(it.value());
            }
            array.append(obj);
        } else {
            array.append(QJsonValue::fromVariant(item));
        }
    }
    groupObj[key] = array;
    m_rootObject[group] = groupObj;

    return true;
}

bool JsonPersister::setVariant(const QString group, const QString key, const QVariant value)
{
    QJsonObject& groupObj = getOrCreateGroup(group);
    groupObj[key] = QJsonValue::fromVariant(value);
    m_rootObject[group] = groupObj;
    return true;
}
