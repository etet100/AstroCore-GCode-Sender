// This file is a part of "G-Pilot GCode Sender" application.
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
    // Light theme colors
    {"backgroundColorLight", QColor("#f3eedb")},
    {"toolColorLight", QColor("#299cb6")},
    {"cursorColorLight", QColor("#ff003f")},
    {"textColorLight", QColor("#000000")},
    {"normalToolpathColorLight", QColor("#900007")},
    {"drawnToolpathColorLight", QColor("#e1afaf")},
    {"hightlightToolpathColorLight", QColor("#a71ba0")},
    {"zMovementColorLight", QColor("#f3f337")},
    {"rapidMovementColorLight", QColor("#9cd2f3")},
    {"startPointColorLight", QColor("#45ab48")},
    {"endPointColorLight", QColor("#000000")},
    {"tableSurfaceGridColorLight", QColor("#00BFFF")},
    // Dark theme colors
    {"backgroundColorDark", QColor("#2a2a2a")},
    {"toolColorDark", QColor("#62cec2")},
    {"cursorColorDark", QColor("#ff5566")},
    {"textColorDark", QColor("#dcdcdc")},
    {"normalToolpathColorDark", QColor("#e68080")},
    {"drawnToolpathColorDark", QColor("#6888a8")},
    {"hightlightToolpathColorDark", QColor("#d470e4")},
    {"zMovementColorDark", QColor("#eeca6a")},
    {"rapidMovementColorDark", QColor("#78b8f0")},
    {"startPointColorDark", QColor("#96d870")},
    {"endPointColorDark", QColor("#e8a84e")},
    {"tableSurfaceGridColorDark", QColor("#507a8c")},
    //
    {"toolDiameter", 3.175},
    {"toolLength", 30.0},
    {"toolType", ConfigurationVisualizer::ToolType::Modern},
    {"toolAngle", 15.0},
    {"show3dCursor", false},
    {"viewMode", ConfigurationVisualizer::ViewMode::Perspective},
};

ConfigurationVisualizer::ConfigurationVisualizer(QObject *parent) : AbstractConfigurationModule(parent, DEFAULTS)
{
}
