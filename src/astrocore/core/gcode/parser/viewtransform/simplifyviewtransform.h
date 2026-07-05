// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef SIMPLIFYVIEWTRANSFORM_H
#define SIMPLIFYVIEWTRANSFORM_H

#include "abstractviewtransform.h"

class SimplifyViewTransform : public AbstractViewTransform
{
    public:
        explicit SimplifyViewTransform(double precision, double collinearTolerance = 1e-4);

        QList<LineSegment> apply(const QList<LineSegment>& input) const override;

        double precision() const { return m_precision; }
        void setPrecision(double precision) { m_precision = precision; }

        double collinearTolerance() const { return m_collinearTolerance; }
        void setCollinearTolerance(double tolerance) { m_collinearTolerance = tolerance; }

    private:
        double m_precision;
        double m_collinearTolerance;
};

#endif // SIMPLIFYVIEWTRANSFORM_H
