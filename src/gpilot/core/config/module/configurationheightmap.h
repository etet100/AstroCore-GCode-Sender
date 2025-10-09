// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONHEIGHTMAP_H
#define CONFIGURATIONHEIGHTMAP_H

#include "configurationmodule.h"

class ConfigurationHeightmap : public ConfigurationModule
{
    Q_OBJECT
    Q_PROPERTY(double borderX MEMBER m_borderX NOTIFY changed)
    Q_PROPERTY(double borderY MEMBER m_borderY NOTIFY changed)
    Q_PROPERTY(double borderWidth MEMBER m_borderWidth NOTIFY changed)
    Q_PROPERTY(double borderHeight MEMBER m_borderHeight NOTIFY changed)
    Q_PROPERTY(bool borderShow MEMBER m_borderShow NOTIFY changed)
    Q_PROPERTY(double gridX MEMBER m_gridX NOTIFY changed)
    Q_PROPERTY(double gridY MEMBER m_gridY NOTIFY changed)
    Q_PROPERTY(double gridZTop MEMBER m_gridZTop NOTIFY changed)
    Q_PROPERTY(double gridZBottom MEMBER m_gridZBottom NOTIFY changed)
    Q_PROPERTY(double probeFeed MEMBER m_probeFeed NOTIFY changed)
    Q_PROPERTY(bool gridShow MEMBER m_gridShow NOTIFY changed)
    Q_PROPERTY(double interpolationStepX MEMBER m_interpolationStepX NOTIFY changed)
    Q_PROPERTY(double interpolationStepY MEMBER m_interpolationStepY NOTIFY changed)
    Q_PROPERTY(int interpolationType MEMBER m_interpolationType NOTIFY changed)
    Q_PROPERTY(bool interpolationShow MEMBER m_interpolationShow NOTIFY changed)

    public:
        ConfigurationHeightmap(QObject *parent);

        QString getSectionName() override { return "heightmap"; }

        double borderX() const { return m_borderX; }
        double borderY() const { return m_borderY; }
        double borderWidth() const { return m_borderWidth; }
        double borderHeight() const { return m_borderHeight; }
        bool borderShow() const { return m_borderShow; }

        double gridX() const { return m_gridX; }
        double gridY() const { return m_gridY; }
        double gridZTop() const { return m_gridZTop; }
        double gridZBottom() const { return m_gridZBottom; }
        double probeFeed() const { return m_probeFeed; }
        bool gridShow() const { return m_gridShow; }

        double interpolationStepX() const { return m_interpolationStepX; }
        double interpolationStepY() const { return m_interpolationStepY; }
        int interpolationType() const { return m_interpolationType; }
        bool interpolationShow() const { return m_interpolationShow; }

    private:
        double m_borderX;
        double m_borderY;
        double m_borderWidth;
        double m_borderHeight;
        bool m_borderShow;

        double m_gridX;
        double m_gridY;
        double m_gridZTop;
        double m_gridZBottom;
        double m_probeFeed;
        bool m_gridShow;

        double m_interpolationStepX;
        double m_interpolationStepY;
        int m_interpolationType;
        bool m_interpolationShow;
};

#endif // CONFIGURATIONHEIGHTMAP_H
