// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef APPLYHEIGHTMAP_H
#define APPLYHEIGHTMAP_H

#include "streamconverter.h"
#include "core/heightmap/heightmap.h"
#include "core/heightmap/interpolator/abstractheightmapinterpolator.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QVector3D>

/**
 * Segments movement lines and applies a heightmap-driven Z offset to each
 * intermediate point. Used to compensate for an uneven work surface
 * (typical case: PCB isolation milling).
 *
 * Stream contract: 1 input movement line yields N output segments.
 * Non-movement lines pass through unchanged. The internal parser is fed
 * with raw (input) coordinates so each new line starts from the
 * un-offset position — preventing double application.
 */
class ApplyHeightmap : public StreamConverter
{
    public:
        // Schema covers only the values the user chooses in UI.
        // The Heightmap pointer is supplied by the caller, not by the dialog.
        static QString parameterSchema();

        explicit ApplyHeightmap(Heightmap* heightmap, double segmentLength = 1.0,
                                bool applyToRapids = false);
        ~ApplyHeightmap() override;

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override;

        void setSegmentLength(double length) { m_segmentLength = length; }
        double segmentLength() const { return m_segmentLength; }

        void setApplyToRapids(bool apply) { m_applyToRapids = apply; }
        bool applyToRapids() const { return m_applyToRapids; }

        // Max allowed Z-offset variation (mm) for arc preservation.
        // If all sample points along a G17 arc get a Z offset within this range,
        // the arc is kept as G2/G3 instead of being segmented into G1 lines.
        void setArcPreserveTolerance(double tolerance) { m_arcPreserveTolerance = tolerance; }
        double arcPreserveTolerance() const { return m_arcPreserveTolerance; }

    private:
        Heightmap* m_heightmap;
        AbstractHeightmapInterpolator* m_interpolator;
        GcodeParser* m_parser;
        double m_segmentLength;
        double m_arcPreserveTolerance;
        bool   m_applyToRapids;

        QList<QVector3D> segmentLine(const QVector3D &start, const QVector3D &end);
        QList<QVector3D> segmentArc(const QVector3D &start, const QVector3D &end,
                                     const QVector3D &center, double radius,
                                     bool clockwise, PointSegment::planes plane);
        void applyHeightmapToPoint(QVector3D &point);
        QString generateGCodeLine(const QVector3D &start, const QVector3D &end,
                                  const GCodeItem &originalItem, bool isFirstSegment,
                                  const QString &commandOverride = QString());

        // Returns true if Z offsets along the arc are uniform within tolerance.
        // avgOffset receives the average offset to apply to Z_end.
        bool checkArcZOffsetUniform(const QVector3D &start, const QVector3D &end,
                                    const QVector3D &center, double radius,
                                    bool clockwise, double &avgOffset) const;

        // Returns arc line with Z_end adjusted by zOffset.
        // If the original line has no Z (non-helical arc), it is returned unchanged.
        QString adjustZInArcLine(const GCodeItem &item, double originalEndZ, double zOffset) const;
};

#endif // APPLYHEIGHTMAP_H
