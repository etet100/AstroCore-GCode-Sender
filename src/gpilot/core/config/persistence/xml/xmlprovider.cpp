#include "xmlprovider.h"
#include "../../../globals.h"
#include "qguiapplication.h"
#include <QDebug>

XmlProvider::XmlProvider(QObject *parent, const QString &filePath) : Provider(parent), m_filePath(filePath)
{
    m_settings = nullptr;
}

bool XmlProvider::open()
{
    if (m_settings) {
        return false;
    }

    m_settings = new QSettings(m_filePath, QSettings::CustomFormat1);

    return true;
}

void XmlProvider::close()
{
    if (!m_settings) {
        return;
    }

    delete m_settings;
    m_settings = nullptr;
}

int XmlProvider::getInt(const QString group, const QString key, int defaultValue)
{
    return m_settings->value(group + '\\' + key, defaultValue).toInt();
}

bool XmlProvider::getBool(const QString group, const QString key, bool defaultValue)
{
    return m_settings->value(group + '\\' + key, defaultValue).toBool();
}

QString XmlProvider::getString(const QString group, const QString key, QString defaultValue)
{
    return m_settings->value(group + '\\' + key, defaultValue).toString();
}

double XmlProvider::getDouble(const QString group, const QString key, double defaultValue)
{
    return m_settings->value(group + '\\' + key, defaultValue).toDouble();
}

QVariant XmlProvider::getVariant(const QString group, const QString key, QVariant defaultValue)
{
    return m_settings->value(group + '\\' + key, defaultValue);
}
