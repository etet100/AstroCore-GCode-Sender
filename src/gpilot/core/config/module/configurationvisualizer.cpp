// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationvisualizer.h"

const QMap<QString, QVariant> DEFAULTS = {
    {"lineWidth", 1.5},
    {"fpsLock", 60},
    {"antialiasing", true},
    {"msaa", true},
    {"zBuffer", false},
    {"vsync", false},
    {"simplifyGeometry", false},
    {"simplifyGeometryPrecision", 1},
    {"grayscaleSegments", false},
    {"grayscaleSegmentsBySCode", true},
    {"grayscaleSegmentsByZCode", false},
    {"fieldOfView", 60},
    {"nearPlane", 0.5},
    {"farPlane", 10000.0},
    {"backgroundColor", QColor("#f3eedb")},
    {"toolColor", QColor("#299cb6")},
    {"textColor", QColor("#000000")},
    {"normalToolpathColor", QColor("#900007")},
    {"drawnToolpathColor", QColor("#e1afaf")},
    {"hightlightToolpathColor", QColor("#a71ba0")},
    {"zMovementColor", QColor("#f3f337")},
    {"rapidMovementColor", QColor("#9cd2f3")},
    {"startPointColor", QColor("#45ab48")},
    {"endPointColor", QColor("#000000")},
    {"tableSurfaceGridColor", QColor("#00BFFF")},
    {"toolDiameter", 3.175},
    {"toolLength", 30.0},
    {"toolType", ConfigurationVisualizer::ToolType::Flat},
    {"toolAngle", 15.0},
    {"show3dCursor", false},
};

ConfigurationVisualizer::ConfigurationVisualizer(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}
