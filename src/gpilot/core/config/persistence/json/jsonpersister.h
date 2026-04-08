#ifndef JSON_CONFIG_PERSISTER_H
#define JSON_CONFIG_PERSISTER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include "../persister.h"

class JsonPersister : public Persister
{
    public:
        JsonPersister(QObject *parent, const QString &filePath);
        bool open() override;
        void close() override;
        bool setInt(const QString group, const QString key, const int value) override;
        bool setString(const QString group, const QString key, const QString value) override;
        bool setDouble(const QString group, const QString key, const double value) override;
        bool setBool(const QString group, const QString key, const bool value) override;
        bool setStringList(const QString group, const QString key, const QStringList value) override;
        bool setVariantMap(const QString group, const QString key, const QVariantMap value) override;
        bool setVariantList(const QString group, const QString key, const QVariantList value) override;
        bool setVariant(const QString group, const QString key, const QVariant value) override;

    private:
        QJsonObject m_rootObject;
        QString m_filePath;
        bool saveDocument();
        QJsonObject& getOrCreateGroup(const QString& group);
};

#endif // JSON_CONFIG_PERSISTER_H
