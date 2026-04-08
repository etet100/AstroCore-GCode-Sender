#include "iniprovider.h"
#include "core/globals.h"
#include "qguiapplication.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

IniProvider::IniProvider(QObject *parent, const QString &filePath) : Provider(parent), m_filePath(filePath)
{
    m_settings = nullptr;
}

bool IniProvider::open()
{
    if (m_settings) {
        return false;
    }

    qDebug() << "[Configuration] Open file: " << m_filePath;

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
    QStringList list = getVariant(group, key, defaultValue).toStringList();

    // do not return list with single empty string
    if (list.length() == 1 && list[0] == "") {
        return QStringList();
    }

    return list;
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

QVariantList IniProvider::getVariantList(const QString group, const QString key, QVariantList defaultValue)
{
    int size = m_settings->beginReadArray(group + "/" + key);
    if (size == 0) {
        m_settings->endArray();

        return defaultValue;
    }

    QVariantList result;
    for (int i = 0; i < size; ++i) {
        m_settings->setArrayIndex(i);
        QStringList keys = m_settings->childKeys();
        if (keys.size() == 1 && keys.first() == "value") {
            result.append(m_settings->value("value"));
        } else {
            QVariantMap map;
            for (const QString& k : keys) {
                map[k] = m_settings->value(k);
            }
            result.append(map);
        }
    }
    m_settings->endArray();

    return result;
}
