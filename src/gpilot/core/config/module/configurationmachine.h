// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONMACHINE_H
#define CONFIGURATIONMACHINE_H

#include <QObject>
#include "configurationmodule.h"
#include <QVector3D>

class ConfigurationMachine : public ConfigurationModule
{
    friend class frmSettings;

    Q_OBJECT
    Q_PROPERTY(ConfigurationModule::MinMax spindleSpeedRange MEMBER m_spindleSpeedRange NOTIFY changed)
    Q_PROPERTY(int spindleSpeed READ spindleSpeed NOTIFY changed)
    Q_PROPERTY(ConfigurationModule::MinMax laserPowerRange MEMBER m_laserPowerRange NOTIFY changed)
    Q_PROPERTY(ReferencePositionDir referencePositionDirX MEMBER m_referencePositionDirX NOTIFY changed)
    Q_PROPERTY(ReferencePositionDir referencePositionDirY MEMBER m_referencePositionDirY NOTIFY changed)
    Q_PROPERTY(ReferencePositionDir referencePositionDirZ MEMBER m_referencePositionDirZ NOTIFY changed)
    Q_PROPERTY(bool overrideMaxTravel MEMBER m_overrideMaxTravel NOTIFY changed)
    Q_PROPERTY(QVector3D maxTravel MEMBER m_maxTravel NOTIFY changed)
    Q_PROPERTY(bool overrideFeed MEMBER m_overrideFeed NOTIFY changed)
    Q_PROPERTY(int overrideFeedValue MEMBER m_overrideFeedValue NOTIFY changed)
    Q_PROPERTY(bool overrideRapid MEMBER m_overrideRapid NOTIFY changed)
    Q_PROPERTY(int overrideRapidValue MEMBER m_overrideRapidValue NOTIFY changed)
    Q_PROPERTY(bool overrideSpindleSpeed MEMBER m_overrideSpindleSpeed NOTIFY changed)
    Q_PROPERTY(int overrideSpindleSpeedValue MEMBER m_overrideSpindleSpeedValue NOTIFY changed)

    public:
        ConfigurationMachine(QObject *parent);
        QString getSectionName() override { return "machine"; }

        enum ReferencePositionDir : int {
            Negative,
            Positive
        };
        Q_ENUM(ReferencePositionDir);

        ConfigurationModule::MinMax spindleSpeedRange() const { return m_spindleSpeedRange; }
        double spindleSpeedRatio() const { return (m_spindleSpeedRange.max - m_spindleSpeedRange.min) / 100; }
        int spindleSpeed() const { return m_spindleSpeed; }
        void setSpindleSpeed(int spindleSpeed) { m_spindleSpeed = spindleSpeed; emit changed(); }
        ConfigurationModule::MinMax laserPowerRange() const { return m_laserPowerRange; }
        ReferencePositionDir referencePositionDirX() const { return m_referencePositionDirX; }
        ReferencePositionDir referencePositionDirY() const { return m_referencePositionDirY; }
        ReferencePositionDir referencePositionDirZ() const { return m_referencePositionDirZ; }
        bool overrideMaxTravel() const { return m_overrideMaxTravel; }
        QVector3D maxTravel() const { return m_maxTravel; }
        bool overrideFeed() const { return m_overrideFeed; }
        int overrideFeedValue() const { return m_overrideFeedValue; }
        bool overrideRapid() const { return m_overrideRapid; }
        int overrideRapidValue() const { return m_overrideRapidValue; }
        bool overrideSpindleSpeed() const { return m_overrideSpindleSpeed; }
        int overrideSpindleSpeedValue() const { return m_overrideSpindleSpeedValue; }

    private:
        MinMax m_spindleSpeedRange;
        int m_spindleSpeed;
        MinMax m_laserPowerRange;
        ReferencePositionDir m_referencePositionDirX;
        ReferencePositionDir m_referencePositionDirY;
        ReferencePositionDir m_referencePositionDirZ;
        bool m_overrideMaxTravel;
        QVector3D m_maxTravel;
        bool m_overrideFeed;
        int m_overrideFeedValue;
        bool m_overrideRapid;
        int m_overrideRapidValue;
        bool m_overrideSpindleSpeed;
        int m_overrideSpindleSpeedValue;
};

#endif // CONFIGURATIONMACHINE_H
