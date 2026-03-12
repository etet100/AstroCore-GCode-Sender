// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef APPLYHEIGHTMAP_H
#define APPLYHEIGHTMAP_H

#include "converterinterface.h"
#include "core/heightmap/heightmap.h"
#include "core/heightmap/interpolator/heightmapinterpolator.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QVector3D>
#include <QObject>

/**
 * ApplyHeightmap is a specialized converter that segments movement lines
 * and applies Z-offset from heightmap. Due to line insertion requirement,
 * it implements ConverterInterface directly rather than using Converter base.
 *
 * IMPORTANT: Do not use in Pipeline with other converters. Use standalone:
 *   ApplyHeightmap converter(heightmap, 1.0);
 *   converter.setGCode(originalGCode);
 *   GCode* result = converter.convertAll();
 */
class ApplyHeightmap : public QObject, public ConverterInterface
{
    Q_OBJECT

    public:
        explicit ApplyHeightmap(Heightmap* heightmap, double segmentLength = 1.0, QObject *parent = nullptr);
        ~ApplyHeightmap() override;

        // ConverterInterface implementation
        void setGCode(GCode *gcode) override;
        int convertNext(int count) override;
        void reset() override;
        bool hasMore() const override;
        int currentPosition() const override { return m_currentIndex; }
        int totalLines() const override;
        GCode* convertAll() override;

        void setSegmentLength(double length) { m_segmentLength = length; }
        double segmentLength() const { return m_segmentLength; }

    signals:
        void progressChanged(int current, int total);

    private:
        Heightmap* m_heightmap;
        HeightmapInterpolator* m_interpolator;
        GcodeParser* m_parser;
        GCode* m_gcode;
        double m_segmentLength; // max segment length in mm
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
};

#endif // APPLYHEIGHTMAP_H
