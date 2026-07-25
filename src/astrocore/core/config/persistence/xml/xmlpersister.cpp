// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "xmlpersister.h"
#include "core/globals.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QVariant>
#include <QDir>
#include <QDebug>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

XmlPersister::XmlPersister(QObject *parent, const QString &filePath) : AbstractPersister(parent), m_filePath(filePath)
{
}

bool XmlPersister::open()
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
        // Create new document
        m_doc = QDomDocument("config");
        QDomElement root = m_doc.createElement("config");
        m_doc.appendChild(root);
        return true;
    }
}

void XmlPersister::close()
{
    if (!m_doc.isNull()) {
        saveDocument();
    }
    m_doc.clear();
}

bool XmlPersister::saveDocument()
{
    QFile file(m_filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        stream << m_doc.toString(4); // Indented
        file.close();
        return true;
    } else {
        qDebug() << "[Configuration][XML] Failed to open XML configuration file for writing:" << m_filePath;
        return false;
    }
}

QDomElement XmlPersister::getOrCreateGroup(const QString& group)
{
    QDomElement root = m_doc.documentElement();
    if (root.isNull()) {
        root = m_doc.createElement("config");
        m_doc.appendChild(root);
    }

    QDomNodeList groups = root.elementsByTagName("group");
    for (int i = 0; i < groups.size(); ++i) {
        QDomElement g = groups.at(i).toElement();
        if (g.attribute("name") == group) {
            return g;
        }
    }

    // Create new group
    QDomElement newGroup = m_doc.createElement("group");
    newGroup.setAttribute("name", group);
    root.appendChild(newGroup);
    return newGroup;
}

// An entry saved with an empty value has no text node, so setNodeValue() on
// firstChild() would silently do nothing and the new value would be lost.
void XmlPersister::setElementText(QDomElement& element, const QString& text)
{
    QDomNode child = element.firstChild();
    if (child.isNull() || !child.isText()) {
        while (!element.firstChild().isNull()) {
            element.removeChild(element.firstChild());
        }
        element.appendChild(m_doc.createTextNode(text));

        return;
    }

    child.setNodeValue(text);
}

QString XmlPersister::variantToString(const QVariant& value)
{
    if (value.typeId() == QMetaType::QStringList) {
        QStringList list = value.toStringList();
        return list.join(",");
    } else if (value.typeId() == QMetaType::QVariantMap) {
        // For simplicity, serialize as JSON-like string
        QVariantMap map = value.toMap();
        QStringList parts;
        for (auto it = map.begin(); it != map.end(); ++it) {
            parts.append(it.key() + "=" + it.value().toString());
        }
        return parts.join(";");
    } else {
        return value.toString();
    }
}

bool XmlPersister::setInt(const QString group, const QString key, const int value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "int");
            setElementText(entry, QString::number(value));
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "int");
    QDomText text = m_doc.createTextNode(QString::number(value));
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}

bool XmlPersister::setString(const QString group, const QString key, const QString value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "string");
            setElementText(entry, value);
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "string");
    QDomText text = m_doc.createTextNode(value);
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}

bool XmlPersister::setDouble(const QString group, const QString key, const double value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "double");
            setElementText(entry, QString::number(value));
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "double");
    QDomText text = m_doc.createTextNode(QString::number(value));
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}

bool XmlPersister::setBool(const QString group, const QString key, const bool value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "bool");
            setElementText(entry, value ? "true" : "false");
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "bool");
    QDomText text = m_doc.createTextNode(value ? "true" : "false");
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}

bool XmlPersister::setStringList(const QString group, const QString key, const QStringList value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "stringlist");
            setElementText(entry, value.join(","));
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "stringlist");
    QDomText text = m_doc.createTextNode(value.join(","));
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}

bool XmlPersister::setVariantMap(const QString group, const QString key, const QVariantMap value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "variantmap");
            QStringList parts;
            QMapIterator<QString, QVariant> it(value);
            while (it.hasNext()) {
                it.next();
                parts.append(it.key() + "=" + it.value().toString());
            }
            setElementText(entry, parts.join(";"));
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "variantmap");
    QStringList parts;
    QMapIterator<QString, QVariant> it(value);
    while (it.hasNext()) {
        it.next();
        parts.append(it.key() + "=" + it.value().toString());
    }
    QDomText text = m_doc.createTextNode(parts.join(";"));
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}

bool XmlPersister::setVariantList(const QString group, const QString key, const QVariantList value)
{
    QDomElement groupElem = getOrCreateGroup(group);

    // Remove existing list element
    QDomNodeList lists = groupElem.elementsByTagName("list");
    for (int i = 0; i < lists.size(); ++i) {
        QDomElement list = lists.at(i).toElement();
        if (list.attribute("key") == key) {
            groupElem.removeChild(list);

            break;
        }
    }

    QDomElement listElem = m_doc.createElement("list");
    listElem.setAttribute("key", key);

    for (const QVariant& item : value) {
        QDomElement itemElem = m_doc.createElement("item");
        if (item.typeId() == QMetaType::QVariantMap) {
            QVariantMap map = item.toMap();
            for (auto it = map.begin(); it != map.end(); ++it) {
                QDomElement entryElem = m_doc.createElement("entry");
                entryElem.setAttribute("key", it.key());
                QString typeStr;
                switch (it.value().typeId()) {
                    case QMetaType::Int: typeStr = "int"; break;
                    case QMetaType::Bool: typeStr = "bool"; break;
                    case QMetaType::Double: typeStr = "double"; break;
                    default: typeStr = "string"; break;
                }
                entryElem.setAttribute("type", typeStr);
                QString valueStr = (it.value().typeId() == QMetaType::Bool)
                    ? (it.value().toBool() ? "true" : "false")
                    : it.value().toString();
                QDomText text = m_doc.createTextNode(valueStr);
                entryElem.appendChild(text);
                itemElem.appendChild(entryElem);
            }
        } else {
            QDomText text = m_doc.createTextNode(item.toString());
            itemElem.appendChild(text);
        }
        listElem.appendChild(itemElem);
    }

    groupElem.appendChild(listElem);

    return true;
}

bool XmlPersister::setVariant(const QString group, const QString key, const QVariant value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "variant");
            setElementText(entry, variantToString(value));
            return true;
        }
    }

    // Create new entry
    QDomElement newEntry = m_doc.createElement("entry");
    newEntry.setAttribute("key", key);
    newEntry.setAttribute("type", "variant");
    QDomText text = m_doc.createTextNode(variantToString(value));
    newEntry.appendChild(text);
    groupElem.appendChild(newEntry);
    return true;
}
