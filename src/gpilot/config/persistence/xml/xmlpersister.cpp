#include "xmlpersister.h"
#include "../../../globals.h"
#include "qguiapplication.h"

XmlPersister::XmlPersister(QObject *parent) : Persister(parent)
{
    m_settings = nullptr;
}

bool XmlPersister::open()
{
    if (m_settings) {
        return false;
    }

    m_settings = new QSettings(qApp->applicationDirPath() + "/" + CONFIGURATION_FILE, QSettings::CustomFormat1);

    return true;
}

void XmlPersister::close()
{
    if (!m_settings) {
        return;
    }

    delete m_settings;
    m_settings = nullptr;
}

bool XmlPersister::setInt(const QString group, const QString key, const int value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + '/' + key, value);

    return true;
}

bool XmlPersister::setString(const QString group, const QString key, const QString value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + '/' + key, value);

    return true;
}

bool XmlPersister::setDouble(const QString group, const QString key, const double value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + '/' + key, value);

    return true;
}

bool XmlPersister::setBool(const QString group, const QString key, const bool value)
{
    if (!m_settings) {
        return false;
    }
    m_settings->setValue(group + '/' + key, value);

    return true;
}
