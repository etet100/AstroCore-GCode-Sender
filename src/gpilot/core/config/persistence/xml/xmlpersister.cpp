#include "xmlpersister.h"
#include "../../../globals.h"
#include <QGuiApplication>
#include <QJsonArray>
#include <QVariant>
#include <QDir>
#include <QDebug>
#include <QTextStream>

XmlPersister::XmlPersister(QObject *parent, const QString &filePath) : Persister(parent), m_filePath(filePath)
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
                qDebug() << "Failed to parse XML configuration file:" << m_filePath;
                file.close();
                return false;
            }
        } else {
            qDebug() << "Failed to open XML configuration file for reading:" << m_filePath;
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
        qDebug() << "Failed to open XML configuration file for writing:" << m_filePath;
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
            entry.firstChild().setNodeValue(QString::number(value));
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
            entry.firstChild().setNodeValue(value);
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
            entry.firstChild().setNodeValue(QString::number(value));
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
            entry.firstChild().setNodeValue(value ? "true" : "false");
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
            entry.firstChild().setNodeValue(value.join(","));
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
            entry.firstChild().setNodeValue(parts.join(";"));
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

bool XmlPersister::setVariant(const QString group, const QString key, const QVariant value)
{
    QDomElement groupElem = getOrCreateGroup(group);
    QDomNodeList entries = groupElem.elementsByTagName("entry");
    for (int i = 0; i < entries.size(); ++i) {
        QDomElement entry = entries.at(i).toElement();
        if (entry.attribute("key") == key) {
            entry.setAttribute("type", "variant");
            entry.firstChild().setNodeValue(variantToString(value));
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
