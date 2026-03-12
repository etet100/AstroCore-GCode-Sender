// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef ARCSTOLINES_H
#define ARCSTOLINES_H

#include "converter.h"

/**
 * Converts G2/G3 arc movements into chains of G1 linear segments.
 *
 * arcDegreeMode = false  ->  arcPrecision is max chord deviation in mm
 * arcDegreeMode = true   ->  arcPrecision is degrees per segment
 *
 * Output is always in absolute (G90) coordinates.
 */
class ArcsToLines : public Converter
{
public:
    explicit ArcsToLines(double arcPrecision = 0.1, bool arcDegreeMode = false);

    bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
    bool needsParser() const override { return true; }

    void setArcPrecision(double precision) { m_arcPrecision = precision; }
    double arcPrecision() const { return m_arcPrecision; }

    void setArcDegreeMode(bool degreeMode) { m_arcDegreeMode = degreeMode; }
    bool arcDegreeMode() const { return m_arcDegreeMode; }

private:
    double m_arcPrecision;
    bool m_arcDegreeMode;

    QString buildG1Line(const QVector3D &start, const QVector3D &end,
                        const GCodeItem &source, bool includeF);
};

#endif // ARCSTOLINES_H
