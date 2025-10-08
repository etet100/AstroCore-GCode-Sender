#ifndef INI_CONFIG_PROVIDER_H
#define INI_CONFIG_PROVIDER_H

#include <QSettings>
#include <QObject>
#include "../provider.h"

class IniProvider : public Provider
{
    public:
        IniProvider(QObject *parent);
        bool open() override;
        void close() override;
        int getInt(const QString group, const QString key, int defaultValue) override;
        bool getBool(const QString group, const QString key, bool defaultValue) override;
        QString getString(const QString group, const QString key, QString defaultValue) override;
        double getDouble(const QString group, const QString key, double defaultValue) override;
        QVariant getVariant(const QString group, const QString key, QVariant defaultValue) override;
        QStringList getStringList(const QString group, const QString key, QStringList defaultValue) override;
        QVariantMap getVariantMap(const QString group, const QString key, QVariantMap defaultValue) override;

    private:
        QSettings *m_settings;
};

#endif // INI_CONFIG_PROVIDER_H
