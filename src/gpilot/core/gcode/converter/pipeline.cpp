// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "pipeline.h"

Pipeline::Pipeline() {}

Pipeline &Pipeline::operator<<(Converter &m_converter)
{
    m_converters << &m_converter;

    return *this;
}
