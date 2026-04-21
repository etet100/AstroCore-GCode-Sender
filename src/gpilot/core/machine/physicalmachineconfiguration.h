// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PHYSICALMACHINECONFIGURATION_H
#define PHYSICALMACHINECONFIGURATION_H

#include <QVector3D>
#include <QMap>
#include "core/globals.h"
#include "core/config/module//configurationmachine.h"

class PhysicalMachineConfiguration
{
    public:
        PhysicalMachineConfiguration(QMap<int, double> settings, ConfigurationMachine &configuration);
        bool homingEnabled() { return m_homingEnabled; }
        int axisCount() { return m_axisCount; }
        Units units() { return m_units; }
        bool unitsInches() { return m_units == Units::Inches; }
        bool unitsMM() { return m_units == Units::Millimeters; }
        bool softLimitsEnabled() { return m_softLimitsEnabled; }
        bool hardLimitsEnabled() { return m_hardLimitsEnabled; }
        bool laserMode() { return m_laserMode; }
        double homingPullOff() { return m_homingPullOff; }
        //float rapidSpeed() { return m_rapidSpeed; }
        QVector3D stepsPerMM() { return m_stepsPerMM; }
        QVector3D maxTravel() { return m_maxTravel; }
        QVector3D acceleration() { return m_acceleration; }
        QVector3D maxRate() { return m_maxRate; }
        QVector3D machineBounds() { return m_machineBounds; }
        HomingDirs homingDirs() { return m_homingDirs; }
        QMap<int, double> raw() { return m_raw; }
        double toMetric(double value) const { return m_units == Units::Millimeters ? value : value * 25.4; }
        double toInches(double value) const { return m_units == Units::Inches ? value : value / 25.4; }

    private:
        bool m_homingEnabled = false;
        int m_axisCount = 0;
        Units m_units;
        bool m_softLimitsEnabled = false;
        bool m_hardLimitsEnabled = false;
        bool m_laserMode = false;
        double m_homingPullOff = 0;
        //float m_rapidSpeed = 0;
        QVector3D m_stepsPerMM = QVector3D(0,0,0);
        QVector3D m_maxTravel = QVector3D(0,0,0);
        QVector3D m_acceleration = QVector3D(0,0,0);
        QVector3D m_maxRate = QVector3D(0,0,0);
        QVector3D m_machineBounds = QVector3D(0,0,0);
        HomingDirs m_homingDirs;
        QMap<int, double> m_raw;

        Units setUnits(int setting);
        double negativeValue(double value, bool negative);
};

#endif // PHYSICALMACHINECONFIGURATION_H
