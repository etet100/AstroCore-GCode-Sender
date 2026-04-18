// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef ABSTRACTHEIGHTMAPINTERPOLATOR_H
#define ABSTRACTHEIGHTMAPINTERPOLATOR_H

#include <QPointF>
#include "../heightmap.h"

class AbstractHeightmapInterpolator
{
    public:
        AbstractHeightmapInterpolator(const Heightmap* heightmap);
        virtual ~AbstractHeightmapInterpolator() = default;

        // Get interpolated height at the given (x, y) coordinates in millimeters.
        virtual double interpolate(QPointF ptMm) const = 0;

    protected:
        const Heightmap* m_heightmap;

        QPointF toGridCoord(QPointF ptMm) const {
            const QPointF start = m_heightmap->startPos();
            const QSizeF step = m_heightmap->stepSize();

            return QPointF(
                (ptMm.x() - start.x()) / step.width(),
                (ptMm.y() - start.y()) / step.height()
            );
        }
};

#endif // ABSTRACTHEIGHTMAPINTERPOLATOR_H
