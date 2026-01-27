// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2026 BTS

#ifndef CONFIGURATIONPENDANT_H
#define CONFIGURATIONPENDANT_H

#include <QObject>
#include "configurationmodule.h"

class ConfigurationPendant : public ConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(QString wifiSsid MEMBER m_wifiSsid NOTIFY changed)
    Q_PROPERTY(QString wifiPassword MEMBER m_wifiPassword NOTIFY changed)
    Q_PROPERTY(QString hostIp MEMBER m_hostIp NOTIFY changed)

    public:
        ConfigurationPendant(QObject *parent);
        QString getSectionName() override { return "module.pendant"; }

        QString wifiSsid() const { return m_wifiSsid; }
        QString wifiPassword() const { return m_wifiPassword; }
        QString hostIp() const { return m_hostIp; }

    private:
        QString m_wifiSsid;
        QString m_wifiPassword;
        QString m_hostIp;
};

#endif // CONFIGURATIONPENDANT_H
