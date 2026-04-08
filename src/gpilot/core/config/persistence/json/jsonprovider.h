#ifndef JSON_CONFIG_PROVIDER_H
#define JSON_CONFIG_PROVIDER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "core/config/persistence/provider.h"

class JsonProvider : public Provider
{
    public:
        JsonProvider(QObject *parent, const QString &filePath);
        bool open() override;
        void close() override;
        int getInt(const QString group, const QString key, int defaultValue) override;
        bool getBool(const QString group, const QString key, bool defaultValue) override;
        QString getString(const QString group, const QString key, QString defaultValue) override;
        double getDouble(const QString group, const QString key, double defaultValue) override;
        QVariant getVariant(const QString group, const QString key, QVariant defaultValue) override;
        QStringList getStringList(const QString group, const QString key, QStringList defaultValue) override;
        QVariantMap getVariantMap(const QString group, const QString key, QVariantMap defaultValue) override;
        QVariantList getVariantList(const QString group, const QString key, QVariantList defaultValue) override;

    private:
        QJsonObject m_rootObject;
        QString m_filePath;
        QJsonObject getGroup(const QString& group) const;
};

#endif // JSON_CONFIG_PROVIDER_H
