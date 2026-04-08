// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "xmlprovider.h"
#include "core/globals.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariant>
#include <QDebug>

XmlProvider::XmlProvider(QObject *parent, const QString &filePath) : Provider(parent), m_filePath(filePath)
{
}

bool XmlProvider::open()
{
    QFile file(m_filePath);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly)) {
            if (m_doc.setContent(&file)) {
                file.close();
                return true;
            } else {
                qDebug() << "[Configuration][XML] Failed to parse XML configuration file:" << m_filePath;
                file.close();
                return false;
            }
        } else {
            qDebug() << "[Configuration][XML] Failed to open XML configuration file for reading:" << m_filePath;
            return false;
        }
    } else {
        qDebug() << "[Configuration][XML] XML configuration file does not exist:" << m_filePath;
        m_doc = QDomDocument();
        return true; // Allow running with defaults
    }
}

void XmlProvider::close()
{
    m_doc.clear();
}

QDomElement XmlProvider::getGroup(const QString& group) const
{
    QDomElement root = m_doc.documentElement();
    if (root.isNull()) {
        return QDomElement();
    }

    QDomNodeList groups = root.elementsByTagName("group");
    for (int i = 0; i < groups.size(); ++i) {
        QDomElement g = groups.at(i).toElement();
        if (g.attribute("name") == group) {
            return g;
        }
    }

    return QDomElement();
}

QVariant XmlProvider::stringToVariant(const QString& str, const QString& type)
{
    if (type == "int") {
        return str.toInt();
    } else if (type == "double") {
        return str.toDouble();
    } else if (type == "bool") {
        return str == "true";
    } else if (type == "stringlist") {
        return str.split(",");
    } else if (type == "variantmap") {
        QVariantMap map;
        QStringList parts = str.split(";");
        for (const QString& part : parts) {
            QStringList kv = part.split("=");
            if (kv.size() == 2) {
                map[kv[0]] = kv[1];
            }
        }
        return map;
    } else {
        return str;
    }
}

int XmlProvider::getInt(const QString group, const QString key, int defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key && entry.attribute("type") == "int") {
            return entry.text().toInt();
        }
    }

    return defaultValue;
}

bool XmlProvider::getBool(const QString group, const QString key, bool defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key && entry.attribute("type") == "bool") {
            return entry.text() == "true";
        }
    }

    return defaultValue;
}

QString XmlProvider::getString(const QString group, const QString key, QString defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key && entry.attribute("type") == "string") {
            return entry.text();
        }
    }

    return defaultValue;
}

double XmlProvider::getDouble(const QString group, const QString key, double defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key && entry.attribute("type") == "double") {
            return entry.text().toDouble();
        }
    }

    return defaultValue;
}

QVariant XmlProvider::getVariant(const QString group, const QString key, QVariant defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            QString type = entry.attribute("type");
            QString text = entry.text();
            return stringToVariant(text, type);
        }
    }

    return defaultValue;
}

QStringList XmlProvider::getStringList(const QString group, const QString key, QStringList defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key && entry.attribute("type") == "stringlist") {
            return entry.text().split(",");
        }
    }

    return defaultValue;
}

QVariantMap XmlProvider::getVariantMap(const QString group, const QString key, QVariantMap defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key && entry.attribute("type") == "variantmap") {
            QVariantMap map;
            QStringList parts = entry.text().split(";");
            for (const QString& part : parts) {
                QStringList kv = part.split("=");
                if (kv.size() == 2) {
                    map[kv[0]] = kv[1];
                }
            }
            return map;
        }
    }

    return defaultValue;
}

QVariantList XmlProvider::getVariantList(const QString group, const QString key, QVariantList defaultValue)
{
    QDomElement groupElem = getGroup(group);
    if (groupElem.isNull()) {
        return defaultValue;
    }

    QDomNodeList lists = groupElem.elementsByTagName("list");
    for (int i = 0; i < lists.size(); ++i) {
        QDomElement list = lists.at(i).toElement();
        if (list.attribute("key") == key) {
            QVariantList result;
            QDomNodeList items = list.elementsByTagName("item");
            for (int j = 0; j < items.size(); ++j) {
                QDomElement item = items.at(j).toElement();
                QDomNodeList entries = item.elementsByTagName("entry");
                if (entries.isEmpty()) {
                    result.append(item.text());
                } else {
                    QVariantMap map;
                    for (int k = 0; k < entries.size(); ++k) {
                        QDomElement entry = entries.at(k).toElement();
                        map[entry.attribute("key")] = stringToVariant(entry.text(), entry.attribute("type"));
                    }
                    result.append(map);
                }
            }

            return result;
        }
    }

    return defaultValue;
}
