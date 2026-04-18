// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef ABSTRACTVIEWTRANSFORM_H
#define ABSTRACTVIEWTRANSFORM_H

#include <QList>
#include "core/gcode/parser/linesegment.h"

class AbstractViewTransform
{
    public:
        virtual ~AbstractViewTransform() = default;

        virtual QList<LineSegment> apply(const QList<LineSegment>& input) const = 0;
};

#endif // ABSTRACTVIEWTRANSFORM_H
