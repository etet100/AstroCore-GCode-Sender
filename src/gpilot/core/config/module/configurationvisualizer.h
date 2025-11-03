// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATION_VISUALIZER_H
#define CONFIGURATION_VISUALIZER_H

#include <QObject>
#include <QColor>
#include "configurationmodule.h"

class ConfigurationVisualizer : public ConfigurationModule
{
    friend class frmSettings;

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
    Q_PROPERTY(QColor backgroundColor MEMBER m_backgroundColor NOTIFY changed)
    Q_PROPERTY(QColor toolColor MEMBER m_toolColor NOTIFY changed)
    Q_PROPERTY(QColor cursorColor MEMBER m_cursorColor NOTIFY changed)
    Q_PROPERTY(QColor textColor MEMBER m_textColor NOTIFY changed)
    Q_PROPERTY(QColor normalToolpathColor MEMBER m_normalToolpathColor NOTIFY changed)
    Q_PROPERTY(QColor drawnToolpathColor MEMBER m_drawnToolpathColor NOTIFY changed)
    Q_PROPERTY(QColor hightlightToolpathColor MEMBER m_hightlightToolpathColor NOTIFY changed)
    Q_PROPERTY(QColor zMovementColor MEMBER m_zMovementColor NOTIFY changed)
    Q_PROPERTY(QColor rapidMovementColor MEMBER m_rapidMovementColor NOTIFY changed)
    Q_PROPERTY(QColor startPointColor MEMBER m_startPointColor NOTIFY changed)
    Q_PROPERTY(QColor endPointColor MEMBER m_endPointColor NOTIFY changed)
    Q_PROPERTY(QColor tableSurfaceGridColor MEMBER m_tableSurfaceGridColor NOTIFY changed)
    Q_PROPERTY(double toolDiameter MEMBER m_toolDiameter NOTIFY changed)
    Q_PROPERTY(double toolLength MEMBER m_toolLength NOTIFY changed)
    Q_PROPERTY(ToolType toolType MEMBER m_toolType NOTIFY changed)
    Q_PROPERTY(double toolAngle MEMBER m_toolAngle NOTIFY changed)
    Q_PROPERTY(bool show3dCursor MEMBER m_show3dCursor NOTIFY changed)

    public:
        explicit ConfigurationVisualizer(QObject *parent);
        ConfigurationVisualizer& operator=(const ConfigurationVisualizer&) { return *this; }
        QString getSectionName() override { return "baseui.visualizer"; }

        enum ToolType {
            Flat,
            Conic
        };
        Q_ENUM(ToolType);

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
        // colors
        QColor backgroundColor() const { return m_backgroundColor; }
        QColor toolColor() const { return m_toolColor; }
        QColor cursorColor() const { return m_cursorColor; }
        QColor textColor() const { return m_textColor; }
        QColor normalToolpathColor() const { return m_normalToolpathColor; }
        QColor drawnToolpathColor() const { return m_drawnToolpathColor; }
        QColor hightlightToolpathColor() const { return m_hightlightToolpathColor; }
        QColor zMovementColor() const { return m_zMovementColor; }
        QColor rapidMovementColor() const { return m_rapidMovementColor; }
        QColor startPointColor() const { return m_startPointColor; }
        QColor endPointColor() const { return m_endPointColor; }
        QColor tableSurfaceGridColor() const { return m_tableSurfaceGridColor; }
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
        // colors
        QColor m_backgroundColor;
        QColor m_toolColor;
        QColor m_cursorColor;
        QColor m_textColor;
        QColor m_normalToolpathColor;
        QColor m_drawnToolpathColor;
        QColor m_hightlightToolpathColor;
        QColor m_zMovementColor;
        QColor m_rapidMovementColor;
        QColor m_startPointColor;
        QColor m_endPointColor;
        QColor m_tableSurfaceGridColor;
        // tool
        double m_toolDiameter;
        double m_toolLength;
        ToolType m_toolType;
        double m_toolAngle;
};

#endif // CONFIGURATION_VISUALIZER_H
