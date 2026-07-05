// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#ifndef CONFIGURATIONPENDANT_H
#define CONFIGURATIONPENDANT_H

#include <QObject>
#include "core/config/module/abstractconfigurationmodule.h"

class Configuration;

class ConfigurationPendant : public AbstractConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(QString wifiSsid MEMBER m_wifiSsid NOTIFY changed)
    Q_PROPERTY(QString wifiPassword MEMBER m_wifiPassword NOTIFY changed)
    Q_PROPERTY(QString hostIp MEMBER m_hostIp NOTIFY changed)
    Q_PROPERTY(int port MEMBER m_port NOTIFY changed)
    Q_PROPERTY(bool enabled MEMBER m_enabled NOTIFY changed)

    public:
        ConfigurationPendant(QObject *parent = nullptr);
        QString getSectionName() override { return "module.pendant"; }

        QString wifiSsid() const { return m_wifiSsid; }
        QString wifiPassword() const { return m_wifiPassword; }
        QString hostIp() const { return m_hostIp; }
        int port() const { return m_port; }
        bool enabled() const { return m_enabled; }

        static ConfigurationPendant& instance();
        static void registerWith(Configuration& cfg);

    private:
        QString m_wifiSsid;
        QString m_wifiPassword;
        QString m_hostIp;
        int m_port;
        bool m_enabled;
};

#endif // CONFIGURATIONPENDANT_H
