// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef SINGLECONVERTER_H
#define SINGLECONVERTER_H

#include "converter.h"
#include "converterinterface.h"
#include "core/gcode/parser/gcodeparser.h"

/**
 * Wrapper for using a single Converter with the same interface as Pipeline.
 * Allows treating single converter and pipeline uniformly.
 */
class SingleConverter : public ConverterInterface
{
public:
    explicit SingleConverter(Converter *converter);
    ~SingleConverter();

    void setGCode(GCode *gcode) override;
    int convertNext(int count) override;
    void reset() override;
    bool hasMore() const override;
    int currentPosition() const override { return m_currentIndex; }
    int totalLines() const override;
    GCode* convertAll() override;

private:
    Converter *m_converter;
    GCode *m_gcode;
    GcodeParser *m_parser;
    int m_currentIndex;

    bool processLine(int index);
};

#endif // SINGLECONVERTER_H
