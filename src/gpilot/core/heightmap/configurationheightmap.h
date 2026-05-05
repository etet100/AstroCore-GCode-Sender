// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONHEIGHTMAP_H
#define CONFIGURATIONHEIGHTMAP_H

#include "core/config/module/abstractconfigurationmodule.h"

class Configuration;

class ConfigurationHeightmap : public AbstractConfigurationModule
{
    Q_OBJECT
    Q_PROPERTY(double areaX1 MEMBER m_areaX1 NOTIFY changed)
    Q_PROPERTY(double areaY1 MEMBER m_areaY1 NOTIFY changed)
    Q_PROPERTY(double areaX2 MEMBER m_areaX2 NOTIFY changed)
    Q_PROPERTY(double areaY2 MEMBER m_areaY2 NOTIFY changed)
    Q_PROPERTY(double gridX MEMBER m_gridX NOTIFY changed)
    Q_PROPERTY(double gridY MEMBER m_gridY NOTIFY changed)
    Q_PROPERTY(double gridZTop MEMBER m_gridZTop NOTIFY changed)
    Q_PROPERTY(double gridZBottom MEMBER m_gridZBottom NOTIFY changed)
    Q_PROPERTY(double probeFeed MEMBER m_probeFeed NOTIFY changed)
    Q_PROPERTY(double interpolationStepX MEMBER m_interpolationStepX NOTIFY changed)
    Q_PROPERTY(double interpolationStepY MEMBER m_interpolationStepY NOTIFY changed)
    Q_PROPERTY(int interpolationType MEMBER m_interpolationType NOTIFY changed)

    public:
        ConfigurationHeightmap(QObject *parent = nullptr);

        QString getSectionName() override { return "heightmap"; }

        double areaX1() const { return m_areaX1; }
        double areaY1() const { return m_areaY1; }
        double areaX2() const { return m_areaX2; }
        double areaY2() const { return m_areaY2; }

        double gridX() const { return m_gridX; }
        double gridY() const { return m_gridY; }
        double gridZTop() const { return m_gridZTop; }
        double gridZBottom() const { return m_gridZBottom; }
        double probeFeed() const { return m_probeFeed; }

        double interpolationStepX() const { return m_interpolationStepX; }
        double interpolationStepY() const { return m_interpolationStepY; }
        int interpolationType() const { return m_interpolationType; }

        static ConfigurationHeightmap& instance();
        static void registerWith(Configuration& cfg);

    private:
        double m_areaX1;
        double m_areaY1;
        double m_areaX2;
        double m_areaY2;

        double m_gridX;
        double m_gridY;
        double m_gridZTop;
        double m_gridZBottom;
        double m_probeFeed;

        double m_interpolationStepX;
        double m_interpolationStepY;
        int m_interpolationType;
};

#endif // CONFIGURATIONHEIGHTMAP_H
