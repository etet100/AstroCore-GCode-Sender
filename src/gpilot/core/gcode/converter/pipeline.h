// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PIPELINE_H
#define PIPELINE_H

#include "converter.h"

class Pipeline {
    public:
        Pipeline();
        Pipeline &operator<<(Converter &m_converter);

    private:
        QList<Converter*> m_converters;
};

#endif // PIPELINE_H
