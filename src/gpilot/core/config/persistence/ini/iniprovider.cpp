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
    QVariant raw = m_settings->value(group + "/" + key);
    if (!raw.isValid()) {
        return defaultValue;
    }

    QJsonDocument doc = QJsonDocument::fromJson(raw.toString().toUtf8());
    if (!doc.isArray()) {
        return defaultValue;
    }

    QJsonArray array = doc.array();
    QVariantList result;
    for (const QJsonValue& val : array) {
        if (val.isObject()) {
            QJsonObject obj = val.toObject();
            QVariantMap map;
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                map[it.key()] = it.value().toVariant();
            }
            result.append(map);
        } else {
            result.append(val.toVariant());
        }
    }

    return result;
}
