// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef ARCSTOLINES_H
#define ARCSTOLINES_H

#include "converter.h"

class ArcsToLines : public Converter
{
    public:
        ArcsToLines(GCode &data);

    protected:
        GCode& convert() override;
};

#endif // ARCSTOLINES_H
