#ifndef XML_CONFIG_PERSISTER_H
#define XML_CONFIG_PERSISTER_H

#include <QSettings>
#include <QObject>
#include "../persister.h"

class XmlPersister : public Persister
{
    public:
        XmlPersister(QObject *parent);
        bool open() override;
        void close() override;
        bool setInt(const QString group, const QString key, const int value) override;
        bool setString(const QString group, const QString key, const QString value) override;
        bool setDouble(const QString group, const QString key, const double value) override;
        bool setBool(const QString group, const QString key, const bool value) override;

    private:
        QSettings *m_settings;
};

#endif // XML_CONFIG_PERSISTER_H
