#include "iniprovider.h"
#include "../../../globals.h"
#include "qguiapplication.h"
#include <QDebug>

IniProvider::IniProvider(QObject *parent, const QString &filePath) : Provider(parent), m_filePath(filePath)
{
    m_settings = nullptr;
}

bool IniProvider::open()
{
    if (m_settings) {
        return false;
    }

    qDebug() << "Open configuration file: " << m_filePath;

    m_settings = new QSettings(m_filePath, QSettings::IniFormat);
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
    QVariant var = getVariant(group, key, defaultValue);
    if (var.toString().isEmpty()) {
        return QStringList();
    }

    return var.toStringList();
}

QVariantMap IniProvider::getVariantMap(const QString group, const QString key, QVariantMap mapWithDefaultValues)
{
    QMapIterator<QString, QVariant> it(mapWithDefaultValues);
    QVariantMap result;
    while (it.hasNext()) {
        it.next();
        result[it.key()] = m_settings->value(group + "/" + key + "." + it.key(), it.value());
    }

    return result;
}
