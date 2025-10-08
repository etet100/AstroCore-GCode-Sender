#include "iniprovider.h"
#include "../../../globals.h"
#include "qguiapplication.h"
#include <QDebug>

IniProvider::IniProvider(QObject *parent) : Provider(parent)
{
    m_settings = nullptr;
}

bool IniProvider::open()
{
    if (m_settings) {
        return false;
    }

    qDebug() << "Open configuration file: " << qApp->applicationDirPath() + "/" + CONFIGURATION_FILE;

    m_settings = new QSettings(qApp->applicationDirPath() + "/" + CONFIGURATION_FILE, QSettings::IniFormat);
    //m_settings->setIniCodec("UTF-8");

    return true;
}

void IniProvider::close()
{
    if (!m_settings) {
        return;
    }

    delete m_settings;
    m_settings = nullptr;
}

int IniProvider::getInt(const QString group, const QString key, int defaultValue)
{
    return m_settings->value(group + "/" + key, defaultValue).toInt();
}

bool IniProvider::getBool(const QString group, const QString key, bool defaultValue)
{
    return m_settings->value(group + "/" + key, defaultValue).toBool();
}

QString IniProvider::getString(const QString group, const QString key, QString defaultValue)
{
    // qDebug() << m_settings->allKeys() << m_settings->fileName();
    // qDebug() << group + "/" + key << m_settings->value(group + "/" + key, defaultValue);
    return m_settings->value(group + "/" + key, defaultValue).toString();
}

double IniProvider::getDouble(const QString group, const QString key, double defaultValue)
{
    return m_settings->value(group + "/" + key, defaultValue).toDouble();
}

QVariant IniProvider::getVariant(const QString group, const QString key, QVariant defaultValue)
{
    return m_settings->value(group + "/" + key, defaultValue);
}

QStringList IniProvider::getStringList(const QString group, const QString key, QStringList defaultValue)
{
    return getVariant(group, key, defaultValue).toStringList();
}

QVariantMap IniProvider::getVariantMap(const QString group, const QString key, QVariantMap mapWithDefaultValues)
{
    QMapIterator<QString, QVariant> it(mapWithDefaultValues);
    QVariantMap result;
    while (it.hasNext()) {
        it.next();
        result[it.key()] = m_settings->value(group + "/" + key + "." + it.key(), it.value());
    }

    //qDebug() << "abc" << group << key << mapWithDefaultValues << result;

    return result;
}
