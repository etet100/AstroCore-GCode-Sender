// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "singleconverter.h"

SingleConverter::SingleConverter(Converter *converter)
    : m_converter(converter)
    , m_gcode(nullptr)
    , m_parser(nullptr)
    , m_currentIndex(0)
{
}

SingleConverter::~SingleConverter()
{
    delete m_converter;
    delete m_parser;
}

void SingleConverter::setGCode(GCode *gcode)
{
    m_gcode = gcode;
    reset();
}

void SingleConverter::reset()
{
    m_currentIndex = 0;

    if (m_converter) {
        m_converter->reset();
    }

    if (!m_parser) {
        m_parser = new GcodeParser();
    }
    m_parser->reset();
}

int SingleConverter::convertNext(int count)
{
    if (!m_gcode || !m_converter) {
        return 0;
    }

    int converted = 0;
    int endIndex = qMin(m_currentIndex + count, m_gcode->count());

    for (int i = m_currentIndex; i < endIndex; ++i) {
        processLine(i);
        converted++;
        m_currentIndex++;
    }

    return converted;
}

bool SingleConverter::processLine(int index)
{
    if (index < 0 || index >= m_gcode->count()) {
        return false;
    }

    // Local copy prevents dangling reference if the converter calls gcode->insert(),
    // which may cause QList to reallocate its internal buffer.
    GCodeItem item = (*m_gcode)[index];
    GcodeParser *parserPtr = m_converter->needsParser() ? m_parser : nullptr;

    if (parserPtr) parserPtr->pushState();
    bool modified = m_converter->convertLine(item, m_gcode, index, parserPtr);
    if (parserPtr) parserPtr->popState();

    if (modified) {
        (*m_gcode)[index] = item;
    }

    // Must run for every line so the parser position stays correct for the next call.
    m_parser->addCommand((*m_gcode)[index]);

    return modified;
}

bool SingleConverter::hasMore() const
{
    return m_gcode && m_currentIndex < m_gcode->count();
}

int SingleConverter::totalLines() const
{
    return m_gcode ? m_gcode->count() : 0;
}

GCode* SingleConverter::convertAll()
{
    if (!m_gcode || !m_converter) {
        return nullptr;
    }

    GCode *result = new GCode();
    *result << *m_gcode;
    m_parser->reset();

    for (int i = 0; i < result->count(); ++i) {
        GCodeItem item = (*result)[i]; // local copy - see processLine()
        GcodeParser *parserPtr = m_converter->needsParser() ? m_parser : nullptr;

        if (parserPtr) parserPtr->pushState();
        bool modified = m_converter->convertLine(item, result, i, parserPtr);
        if (parserPtr) parserPtr->popState();

        if (modified) {
            (*result)[i] = item;
        }

        m_parser->addCommand((*result)[i]);
    }

    return result;
}
