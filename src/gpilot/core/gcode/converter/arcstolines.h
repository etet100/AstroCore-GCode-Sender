// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef ARCSTOLINES_H
#define ARCSTOLINES_H

#include "streamconverter.h"
#include <QVector3D>

class GcodeParser;
class GCodeItem;

/**
 * Converts G2/G3 arc movements into chains of G1 linear segments.
 *
 * arcDegreeMode = false  ->  arcPrecision is max chord deviation in mm
 * arcDegreeMode = true   ->  arcPrecision is degrees per segment
 *
 * Output is always in absolute (G90) coordinates.
 *
 * Owns its own GcodeParser to keep track of the current point as the stream
 * advances. Every input line is fed to the parser so subsequent arcs know
 * their start point regardless of whether they ride on a non-arc move.
 */
class ArcsToLines : public StreamConverter
{
    public:
        static QString parameterSchema();

        explicit ArcsToLines(double arcPrecision = 0.1, bool arcDegreeMode = false);
        ~ArcsToLines() override;

        QList<GCodeItem> push(const GCodeItem &input) override;
        QList<GCodeItem> flush() override { return {}; }
        void reset() override;

        void setArcPrecision(double precision) { m_arcPrecision = precision; }
        double arcPrecision() const { return m_arcPrecision; }

        void setArcDegreeMode(bool degreeMode) { m_arcDegreeMode = degreeMode; }
        bool arcDegreeMode() const { return m_arcDegreeMode; }

    private:
        GcodeParser *m_parser;
        double m_arcPrecision;
        bool m_arcDegreeMode;

        QString buildG1Line(const QVector3D &start, const QVector3D &end,
                            const GCodeItem &source, bool includeF);
};

#endif // ARCSTOLINES_H
