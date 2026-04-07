// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_VISUALIZER_H
#define CONFIGURATION_VISUALIZER_H

#include <QObject>
#include <QColor>
#include "configurationmodule.h"

class ConfigurationVisualizer : public ConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(double lineWidth MEMBER m_lineWidth NOTIFY changed)
    Q_PROPERTY(int fpsLock MEMBER m_fpsLock NOTIFY changed)
    Q_PROPERTY(bool antialiasing MEMBER m_antialiasing NOTIFY changed)
    Q_PROPERTY(bool msaa MEMBER m_msaa NOTIFY changed)
    Q_PROPERTY(bool zBuffer MEMBER m_zBuffer NOTIFY changed)
    Q_PROPERTY(bool vsync MEMBER m_vsync NOTIFY changed)
    Q_PROPERTY(bool simplifyGeometry MEMBER m_simplifyGeometry NOTIFY changed)
    Q_PROPERTY(double simplifyGeometryPrecision MEMBER m_simplifyGeometryPrecision NOTIFY changed)
    Q_PROPERTY(bool grayscaleSegments MEMBER m_grayscaleSegments NOTIFY changed)
    Q_PROPERTY(bool grayscaleSegmentsBySCode MEMBER m_grayscaleSegmentsBySCode NOTIFY changed)
    Q_PROPERTY(bool grayscaleSegmentsByZCode MEMBER m_grayscaleSegmentsByZCode NOTIFY changed)
    Q_PROPERTY(double fieldOfView MEMBER m_fieldOfView NOTIFY changed)
    Q_PROPERTY(double nearPlane MEMBER m_nearPlane NOTIFY changed)
    Q_PROPERTY(double farPlane MEMBER m_farPlane NOTIFY changed)
    Q_PROPERTY(QColor backgroundColorLight MEMBER m_backgroundColorLight NOTIFY changed)
    Q_PROPERTY(QColor toolColorLight MEMBER m_toolColorLight NOTIFY changed)
    Q_PROPERTY(QColor cursorColorLight MEMBER m_cursorColorLight NOTIFY changed)
    Q_PROPERTY(QColor normalToolpathColorLight MEMBER m_normalToolpathColorLight NOTIFY changed)
    Q_PROPERTY(QColor drawnToolpathColorLight MEMBER m_drawnToolpathColorLight NOTIFY changed)
    Q_PROPERTY(QColor hightlightToolpathColorLight MEMBER m_hightlightToolpathColorLight NOTIFY changed)
    Q_PROPERTY(QColor zMovementColorLight MEMBER m_zMovementColorLight NOTIFY changed)
    Q_PROPERTY(QColor rapidMovementColorLight MEMBER m_rapidMovementColorLight NOTIFY changed)
    Q_PROPERTY(QColor startPointColorLight MEMBER m_startPointColorLight NOTIFY changed)
    Q_PROPERTY(QColor endPointColorLight MEMBER m_endPointColorLight NOTIFY changed)
    Q_PROPERTY(QColor tableSurfaceGridColorLight MEMBER m_tableSurfaceGridColorLight NOTIFY changed)
    Q_PROPERTY(QColor backgroundColorDark MEMBER m_backgroundColorDark NOTIFY changed)
    Q_PROPERTY(QColor toolColorDark MEMBER m_toolColorDark NOTIFY changed)
    Q_PROPERTY(QColor cursorColorDark MEMBER m_cursorColorDark NOTIFY changed)
    Q_PROPERTY(QColor normalToolpathColorDark MEMBER m_normalToolpathColorDark NOTIFY changed)
    Q_PROPERTY(QColor drawnToolpathColorDark MEMBER m_drawnToolpathColorDark NOTIFY changed)
    Q_PROPERTY(QColor hightlightToolpathColorDark MEMBER m_hightlightToolpathColorDark NOTIFY changed)
    Q_PROPERTY(QColor zMovementColorDark MEMBER m_zMovementColorDark NOTIFY changed)
    Q_PROPERTY(QColor rapidMovementColorDark MEMBER m_rapidMovementColorDark NOTIFY changed)
    Q_PROPERTY(QColor startPointColorDark MEMBER m_startPointColorDark NOTIFY changed)
    Q_PROPERTY(QColor endPointColorDark MEMBER m_endPointColorDark NOTIFY changed)
    Q_PROPERTY(QColor tableSurfaceGridColorDark MEMBER m_tableSurfaceGridColorDark NOTIFY changed)
    Q_PROPERTY(double toolDiameter MEMBER m_toolDiameter NOTIFY changed)
    Q_PROPERTY(double toolLength MEMBER m_toolLength NOTIFY changed)
    Q_PROPERTY(ToolType toolType MEMBER m_toolType NOTIFY changed)
    Q_PROPERTY(double toolAngle MEMBER m_toolAngle NOTIFY changed)
    Q_PROPERTY(bool show3dCursor MEMBER m_show3dCursor NOTIFY changed)
    Q_PROPERTY(ViewMode viewMode MEMBER m_viewMode NOTIFY changed)

    public:
        explicit ConfigurationVisualizer(QObject *parent = nullptr);
        ConfigurationVisualizer& operator=(const ConfigurationVisualizer&) { return *this; }
        QString getSectionName() override { return "baseui.visualizer"; }

        enum ToolType {
            Modern,
            Flat,
            Conic
        };
        Q_ENUM(ToolType);

        enum ViewMode {
            Perspective,
            Orthogonal,
            View2D
        };
        Q_ENUM(ViewMode);

        double lineWidth() const { return m_lineWidth; }
        int fpsLock() const { return m_fpsLock; }
        bool antialiasing() const { return m_antialiasing; }
        bool msaa() const { return m_msaa; }
        bool zBuffer() const { return m_zBuffer; }
        bool vsync() const { return m_vsync; }
        bool simplifyGeometry() const { return m_simplifyGeometry; }
        double simplifyGeometryPrecision() const { return m_simplifyGeometryPrecision; }
        bool grayscaleSegments() const { return m_grayscaleSegments; }
        bool grayscaleSegmentsBySCode() const { return m_grayscaleSegmentsBySCode; }
        bool grayscaleSegmentsByZCode() const { return m_grayscaleSegmentsByZCode; }
        bool ignoreZ() const { return grayscaleSegments(); }
        double fieldOfView() const { return m_fieldOfView; }
        double nearPlane() const { return m_nearPlane; }
        double farPlane() const { return m_farPlane; }
        ViewMode viewMode() const {
            return m_viewMode;
        }
        void setViewMode(ViewMode mode) { m_viewMode = mode; }
        // colors
        struct Colors {
            QColor background;
            QColor tool;
            QColor cursor;
            QColor normalToolpath;
            QColor drawnToolpath;
            QColor hightlightToolpath;
            QColor zMovement;
            QColor rapidMovement;
            QColor startPoint;
            QColor endPoint;
            QColor tableSurfaceGrid;
        };
        struct ColorGroups {
            Colors light;
            Colors dark;
        };
        QColor backgroundColorLight() const { return m_backgroundColorLight; }
        QColor toolColorLight() const { return m_toolColorLight; }
        QColor cursorColorLight() const { return m_cursorColorLight; }
        QColor normalToolpathColorLight() const { return m_normalToolpathColorLight; }
        QColor drawnToolpathColorLight() const { return m_drawnToolpathColorLight; }
        QColor hightlightToolpathColorLight() const { return m_hightlightToolpathColorLight; }
        QColor zMovementColorLight() const { return m_zMovementColorLight; }
        QColor rapidMovementColorLight() const { return m_rapidMovementColorLight; }
        QColor startPointColorLight() const { return m_startPointColorLight; }
        QColor endPointColorLight() const { return m_endPointColorLight; }
        QColor tableSurfaceGridColorLight() const { return m_tableSurfaceGridColorLight; }
        QColor backgroundColorDark() const { return m_backgroundColorDark; }
        QColor toolColorDark() const { return m_toolColorDark; }
        QColor cursorColorDark() const { return m_cursorColorDark; }
        QColor normalToolpathColorDark() const { return m_normalToolpathColorDark; }
        QColor drawnToolpathColorDark() const { return m_drawnToolpathColorDark; }
        QColor hightlightToolpathColorDark() const { return m_hightlightToolpathColorDark; }
        QColor zMovementColorDark() const { return m_zMovementColorDark; }
        QColor rapidMovementColorDark() const { return m_rapidMovementColorDark; }
        QColor startPointColorDark() const { return m_startPointColorDark; }
        QColor endPointColorDark() const { return m_endPointColorDark; }
        QColor tableSurfaceGridColorDark() const { return m_tableSurfaceGridColorDark; }
        ColorGroups colors() const {
            return {
                .light = {
                    .background = m_backgroundColorLight,
                    .tool = m_toolColorLight,
                    .cursor = m_cursorColorLight,
                    .normalToolpath = m_normalToolpathColorLight,
                    .drawnToolpath = m_drawnToolpathColorLight,
                    .hightlightToolpath = m_hightlightToolpathColorLight,
                    .zMovement = m_zMovementColorLight,
                    .rapidMovement = m_rapidMovementColorLight,
                    .startPoint = m_startPointColorLight,
                    .endPoint = m_endPointColorLight,
                    .tableSurfaceGrid = m_tableSurfaceGridColorLight
                },
                .dark = {
                    .background = m_backgroundColorDark,
                    .tool = m_toolColorDark,
                    .cursor = m_cursorColorDark,
                    .normalToolpath = m_normalToolpathColorDark,
                    .drawnToolpath = m_drawnToolpathColorDark,
                    .hightlightToolpath = m_hightlightToolpathColorDark,
                    .zMovement = m_zMovementColorDark,
                    .rapidMovement = m_rapidMovementColorDark,
                    .startPoint = m_startPointColorDark,
                    .endPoint = m_endPointColorDark,
                    .tableSurfaceGrid = m_tableSurfaceGridColorDark
                }
            };
        }
        //
        bool show3dCursor() const { return m_show3dCursor; }
        // tool
        double toolDiameter() const { return m_toolDiameter; }
        double toolLength() const { return m_toolLength; }
        ToolType toolType() const { return m_toolType; }
        double toolAngle() const { return m_toolAngle; }

    private:
        double m_lineWidth;
        int m_fpsLock;
        bool m_antialiasing;
        bool m_msaa;
        bool m_zBuffer;
        bool m_vsync;
        bool m_simplifyGeometry;
        double m_simplifyGeometryPrecision;
        bool m_grayscaleSegments;
        bool m_grayscaleSegmentsBySCode;
        bool m_grayscaleSegmentsByZCode;
        double m_fieldOfView;
        double m_nearPlane;
        double m_farPlane;
        bool m_show3dCursor;
        ViewMode m_viewMode;
        // colors
        QColor m_backgroundColorLight;
        QColor m_toolColorLight;
        QColor m_cursorColorLight;
        QColor m_normalToolpathColorLight;
        QColor m_drawnToolpathColorLight;
        QColor m_hightlightToolpathColorLight;
        QColor m_zMovementColorLight;
        QColor m_rapidMovementColorLight;
        QColor m_startPointColorLight;
        QColor m_endPointColorLight;
        QColor m_tableSurfaceGridColorLight;
        QColor m_backgroundColorDark;
        QColor m_toolColorDark;
        QColor m_cursorColorDark;
        QColor m_normalToolpathColorDark;
        QColor m_drawnToolpathColorDark;
        QColor m_hightlightToolpathColorDark;
        QColor m_zMovementColorDark;
        QColor m_rapidMovementColorDark;
        QColor m_startPointColorDark;
        QColor m_endPointColorDark;
        QColor m_tableSurfaceGridColorDark;
        // tool
        double m_toolDiameter;
        double m_toolLength;
        ToolType m_toolType;
        double m_toolAngle;
};

#endif // CONFIGURATION_VISUALIZER_H
