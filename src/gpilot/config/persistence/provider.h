#ifndef CONFIG_PROVIDER_H
#define CONFIG_PROVIDER_H

#include <QSettings>
#include <QObject>

class Provider : public QObject
{
    Q_OBJECT

    public:
        Provider(QObject *parent) : QObject(parent) {};
        virtual bool open() = 0;
        virtual void close() = 0;
        virtual int getInt(const QString group, const QString key, int defaultValue) = 0;
        virtual bool getBool(const QString group, const QString key, bool defaultValue) = 0;
        virtual QString getString(const QString group, const QString key, QString defaultValue) = 0;
        virtual double getDouble(const QString group, const QString key, double defaultValue) = 0;
        virtual QStringList getStringList(const QString group, const QString key, QStringList defaultValue) = 0;
        virtual QVariant getVariant(const QString group, const QString key, QVariant defaultValue) = 0;
        virtual QVariantMap getVariantMap(const QString group, const QString key, QVariantMap defaultValue) = 0;
        virtual bool isReady() { return true; };
};

#endif // CONFIG_PROVIDER_H
