#include "cache.h"

#include <QDebug>

Cache& Cache::instance()
{
    static Cache instance("logviewer_cache.db");

    return instance;
}

Cache::Cache(const QString& path)
{
    int rc = vedis_open(&m_db, path.toStdString().c_str());
    if (rc != VEDIS_OK) {
        qWarning() << "[Cache] Failed to open:" << path;
        m_db = nullptr;
    }
}

Cache::~Cache()
{
    if (m_db) {
        vedis_close(m_db);
    }
}

bool Cache::isOpen() const
{
    return m_db != nullptr;
}

void Cache::flush()
{
    if (m_db) {
        vedis_commit(m_db);
    }
}

void Cache::transaction()
{
    if (m_db) {
        vedis_begin(m_db);
    }
}

void Cache::set(const QString& key, const QString& value)
{
    if (!m_db) {
        return;
    }
    QByteArray k = key.toUtf8();
    QByteArray v = value.toUtf8();
    vedis_kv_store(m_db, k.constData(), k.size(), v.constData(), v.size());
}

void Cache::set(const QString& key, bool value)
{
    set(key, QString::number(value ? 1 : 0));
}

void Cache::set(const QString& key, int value)
{
    set(key, QString::number(value));
}

QString Cache::get(const QString& key, const QString& defaultValue) const
{
    QString result = fetch(key);

    return result.isEmpty() ? defaultValue : result;
}

bool Cache::getBool(const QString& key, bool defaultValue) const
{
    QString result = fetch(key);
    if (result.isEmpty()) {
        return defaultValue;
    }

    return result == "1";
}

int Cache::getInt(const QString& key, int defaultValue) const
{
    QString result = fetch(key);
    if (result.isEmpty()) {
        return defaultValue;
    }

    bool ok;
    int value = result.toInt(&ok);

    return ok ? value : defaultValue;
}

QString Cache::fetch(const QString& key) const
{
    if (!m_db) {
        return {};
    }
    QByteArray k = key.toUtf8();

    vedis_int64 nLen = 0;
    int rc = vedis_kv_fetch(m_db, k.constData(), k.size(), nullptr, &nLen);
    if (rc != VEDIS_OK || nLen <= 0) {
        return {};
    }

    QByteArray buf(static_cast<int>(nLen), '\0');
    rc = vedis_kv_fetch(m_db, k.constData(), k.size(), buf.data(), &nLen);
    if (rc != VEDIS_OK) {
        return {};
    }

    return QString::fromUtf8(buf.constData(), static_cast<int>(nLen));
}
