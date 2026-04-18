// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef XML_CONFIG_PROVIDER_H
#define XML_CONFIG_PROVIDER_H

#include <QObject>
#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include "core/config/persistence/abstractprovider.h"

class XmlProvider : public AbstractProvider
{
   public:
        XmlProvider(QObject *parent, const QString &filePath);
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
        QDomDocument m_doc;
        QString m_filePath;
        QDomElement getGroup(const QString& group) const;
        QVariant stringToVariant(const QString& str, const QString& type);
};

#endif // XML_CONFIG_PROVIDER_H
