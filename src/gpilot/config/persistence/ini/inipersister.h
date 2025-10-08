#ifndef INI_CONFIG_PERSISTER_H
#define INI_CONFIG_PERSISTER_H

#include <QSettings>
#include <QObject>
#include "../persister.h"

class IniPersister : public Persister
{
    public:
        IniPersister(QObject *parent);
        bool open() override;
        void close() override;
        bool setInt(const QString group, const QString key, const int value) override;
        bool setString(const QString group, const QString key, const QString value) override;
        bool setDouble(const QString group, const QString key, const double value) override;
        bool setBool(const QString group, const QString key, const bool value) override;
        bool setStringList(const QString group, const QString key, const QStringList value) override;
        bool setVariantMap(const QString group, const QString key, const QVariantMap value) override;
        bool setVariant(const QString group, const QString key, const QVariant value) override;

    private:
        QSettings *m_settings;

        // Persister interface
};

#endif // INI_CONFIG_PERSISTER_H
