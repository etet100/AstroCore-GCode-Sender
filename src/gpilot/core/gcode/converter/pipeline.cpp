// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "pipeline.h"

Pipeline::Pipeline(QObject *parent)
    : QObject(parent)
    , Converter()
    , m_gcode(nullptr)
    , m_parser(nullptr)
    , m_currentIndex(0)
{
}

Pipeline::~Pipeline()
{
    qDeleteAll(m_converters);
    m_converters.clear();

    // Clean up parser if we created it
    if (m_parser) {
        delete m_parser;
        m_parser = nullptr;
    }
}

Pipeline &Pipeline::operator<<(Converter *converter)
{
    if (converter) {
        m_converters << converter;
    }
    return *this;
}

bool Pipeline::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    if (m_converters.isEmpty()) {
        return false;
    }

    bool wasModified = false;

    for (Converter *converter : m_converters) {
        GcodeParser *parserPtr = (parser && converter->needsParser()) ? parser : nullptr;

        if (parserPtr) {
            parserPtr->pushState();
        }

        bool modified = converter->convertLine(item, gcode, currentIndex, parserPtr);

        if (modified) {
            wasModified = true;
            if (parserPtr) {
                parserPtr->popState();
                parserPtr->addCommand(item.line);
            }
        } else if (parserPtr) {
            parserPtr->popState();
        }
    }

    return wasModified;
}

bool Pipeline::needsParser() const
{
    for (const Converter *converter : m_converters) {
        if (converter->needsParser()) {
            return true;
        }
    }
    return false;
}

void Pipeline::setGCode(GCode *gcode)
{
    m_gcode = gcode;
    reset();
}

void Pipeline::reset()
{
    m_currentIndex = 0;

    // Reset all converters
    for (Converter *converter : m_converters) {
        converter->reset();
    }

    // Create or reset parser
    if (!m_parser) {
        m_parser = new GcodeParser(this);
    }
    m_parser->reset();
}

int Pipeline::convertNext(int count)
{
    if (!m_gcode || m_converters.isEmpty()) {
        return 0;
    }

    int converted = 0;
    int endIndex = qMin(m_currentIndex + count, m_gcode->count());

    for (int i = m_currentIndex; i < endIndex; ++i) {
        bool modified = processLine(i);

        // If line was modified, reparse it before next converter
        if (modified) {
            reparseLine(i);
        }

        converted++;
        m_currentIndex++;

        // Emit progress every 10 lines or at the end
        if (converted % 10 == 0 || m_currentIndex >= m_gcode->count()) {
            emit progressChanged(m_currentIndex, m_gcode->count());
        }
    }

    return converted;
}

bool Pipeline::processLine(int index)
{
    if (index < 0 || index >= m_gcode->count()) {
        return false;
    }

    GCodeItem &item = (*m_gcode)[index];
    bool wasModified = false;

    for (Converter *converter : m_converters) {
        GcodeParser *parserPtr = converter->needsParser() ? m_parser : nullptr;

        if (parserPtr) {
            parserPtr->pushState();
        }

        bool modified = converter->convertLine(item, m_gcode, index, parserPtr);

        if (modified) {
            wasModified = true;
            if (parserPtr) {
                parserPtr->popState();
            }
            reparseLine(index);
        } else if (parserPtr) {
            parserPtr->popState();
        }
    }

    return wasModified;
}

void Pipeline::reparseLine(int index)
{
    if (!m_parser || index < 0 || index >= m_gcode->count()) {
        return;
    }

    GCodeItem &item = (*m_gcode)[index];
    m_parser->addCommand(item.line);
}

bool Pipeline::hasMore() const
{
    return m_gcode && m_currentIndex < m_gcode->count();
}

int Pipeline::totalLines() const
{
    return m_gcode ? m_gcode->count() : 0;
}

GCode* Pipeline::convertAll()
{
    if (!m_gcode) {
        return nullptr;
    }

    GCode *result = new GCode();
    *result << *m_gcode;

    for (int i = 0; i < result->count(); ++i) {
        GCodeItem &item = (*result)[i];

        for (Converter *converter : m_converters) {
            GcodeParser *parserPtr = converter->needsParser() ? m_parser : nullptr;

            if (parserPtr) {
                parserPtr->pushState();
            }

            bool modified = converter->convertLine(item, result, i, parserPtr);

            if (modified && parserPtr) {
                parserPtr->popState();
                parserPtr->addCommand(item.line);
            } else if (parserPtr) {
                parserPtr->popState();
            }
        }
    }

    return result;
}
