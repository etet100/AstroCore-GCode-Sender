#ifndef CONFIGURATIONUPDATE_H
#define CONFIGURATIONUPDATE_H

#include <QObject>
#include "core/config/module/abstractconfigurationmodule.h"

class Configuration;

class ConfigurationUpdate : public AbstractConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(bool checkForUpdates MEMBER m_checkForUpdates NOTIFY changed)
    Q_PROPERTY(int checkIntervalDays MEMBER m_checkIntervalDays NOTIFY changed)

    public:
        explicit ConfigurationUpdate(QObject *parent = nullptr);
        ConfigurationUpdate& operator=(const ConfigurationUpdate&) { return *this; }
        QString getSectionName() override { return "update"; }

        bool checkForUpdates() const { return m_checkForUpdates; }
        void setCheckForUpdates(bool value) { m_checkForUpdates = value; }

        int checkIntervalDays() const { return m_checkIntervalDays; }
        void setCheckIntervalDays(int value) { m_checkIntervalDays = value; }

        // Global accessor for the single update config instance. Owned by the
        // singleton itself; registers with Configuration via registerWith().
        static ConfigurationUpdate& instance();
        static void registerWith(Configuration& cfg);

    private:
        bool m_checkForUpdates;
        int m_checkIntervalDays;
};

#endif // CONFIGURATIONUPDATE_H
