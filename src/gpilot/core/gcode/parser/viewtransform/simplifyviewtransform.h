// This file is a part of "G-Pilot" application.
// Copyright 2026 BTS

#ifndef SIMPLIFYVIEWTRANSFORM_H
#define SIMPLIFYVIEWTRANSFORM_H

#include "abstractviewtransform.h"

class SimplifyViewTransform : public AbstractViewTransform
{
    public:
        explicit SimplifyViewTransform(double precision);

        QList<LineSegment> apply(const QList<LineSegment>& input) const override;

        double precision() const { return m_precision; }
        void setPrecision(double precision) { m_precision = precision; }

    private:
        double m_precision;
};

#endif // SIMPLIFYVIEWTRANSFORM_H
