// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef APPLYHEIGHTMAP_H
#define APPLYHEIGHTMAP_H

#include "abstractbatchconverter.h"
#include "core/heightmap/heightmap.h"
#include "core/heightmap/interpolator/abstractheightmapinterpolator.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QVector3D>
#include <QObject>

/**
 * ApplyHeightmap is a specialized converter that segments movement lines
 * and applies Z-offset from heightmap. Due to line insertion requirement,
 * it implements AbstractBatchConverter directly rather than using AbstractConverter base.
 *
 * IMPORTANT: Do not use in Pipeline with other converters. Use standalone:
 *   ApplyHeightmap converter(heightmap, 1.0);
 *   converter.setGCode(originalGCode);
 *   GCode* result = converter.convertAll();
 */
class ApplyHeightmap : public QObject, public AbstractBatchConverter
{
    Q_OBJECT

    public:
        explicit ApplyHeightmap(Heightmap* heightmap, double segmentLength = 1.0,
                                bool applyToRapids = false, QObject *parent = nullptr);
        ~ApplyHeightmap() override;

        // AbstractBatchConverter implementation
        void setGCode(GCode *gcode) override;
        int convertNext(int count) override;
        void reset() override;
        bool hasMore() const override;
        int currentPosition() const override { return m_currentIndex; }
        int totalLines() const override;
        GCode* convertAll() override;

        void setSegmentLength(double length) { m_segmentLength = length; }
        double segmentLength() const { return m_segmentLength; }

        void setApplyToRapids(bool apply) { m_applyToRapids = apply; }
        bool applyToRapids() const { return m_applyToRapids; }

        // Max allowed Z-offset variation (mm) for arc preservation.
        // If all sample points along a G17 arc get a Z offset within this range,
        // the arc is kept as G2/G3 instead of being segmented into G1 lines.
        void setArcPreserveTolerance(double tolerance) { m_arcPreserveTolerance = tolerance; }
        double arcPreserveTolerance() const { return m_arcPreserveTolerance; }

    signals:
        void progressChanged(int current, int total);

    private:
        Heightmap* m_heightmap;
        AbstractHeightmapInterpolator* m_interpolator;
        GcodeParser* m_parser;
        GCode* m_gcode;
        double m_segmentLength;
        double m_arcPreserveTolerance;
        bool   m_applyToRapids;
        int m_currentIndex;

        // Helper methods for processing single line
        QList<GCodeItem> processLine(const GCodeItem &item);
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
