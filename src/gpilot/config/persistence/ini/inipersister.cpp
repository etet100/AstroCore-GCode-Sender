#include "inipersister.h"
#include "../../../globals.h"
#include <QJsonDocument>
#include <QGuiApplication>
#include <QStringList>
#include <QJsonObject>

IniPersister::IniPersister(QObject *parent) : Persister(parent)
{
    m_settings = nullptr;
}

bool IniPersister::open()
{
    if (m_settings) {
        return false;
    }

    m_settings = new QSettings(qApp->applicationDirPath() + "/" + CONFIGURATION_FILE, QSettings::IniFormat);
    //m_settings->setIniCodec("UTF-8");

    return true;
}

void IniPersister::close()
{
    if (!m_settings) {
        return;
    }

    delete m_settings;
    m_settings = nullptr;
}

bool IniPersister::setInt(const QString group, const QString key, const int value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + "/" + key, value);

    return true;
}

bool IniPersister::setString(const QString group, const QString key, const QString value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + "/" + key, value);

    return true;
}

bool IniPersister::setDouble(const QString group, const QString key, const double value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + "/" + key, value);

    return true;
}

bool IniPersister::setBool(const QString group, const QString key, const bool value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + "/" + key, value);

    return true;
}

bool IniPersister::setStringList(const QString group, const QString key, const QStringList value)
{
    if (!m_settings) {
        return false;
    }

    QStringList list = value;

    m_settings->setValue(group + "/" + key, list);//.replaceInStrings(",", "\\,").join(","));

    return true;
}

bool IniPersister::setVariantMap(const QString group, const QString key, const QVariantMap value)
{
    if (!m_settings) {
        return false;
    }

    QJsonObject obj;
    QMapIterator<QString, QVariant> it(value);
    while (it.hasNext()) {
        it.next();
        obj[it.key()] = it.value().toJsonValue();
        m_settings->setValue(group + "/" + key + "." + it.key(), it.value());
    }
    QJsonDocument doc(obj);

    m_settings->setValue(group + "/" + key, QString(doc.toJson(QJsonDocument::Compact)));

    return true;
}

bool IniPersister::setVariant(const QString group, const QString key, const QVariant value)
{
    if (!m_settings) {
        return false;
    }

    m_settings->setValue(group + "/" + key, value);

    return true;
}
