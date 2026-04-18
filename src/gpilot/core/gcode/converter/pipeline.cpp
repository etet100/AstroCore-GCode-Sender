// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "pipeline.h"

Pipeline::Pipeline(QObject *parent)
    : QObject(parent)
    , AbstractConverter()
    , m_gcode(nullptr)
    , m_parser(nullptr)
    , m_currentIndex(0)
{
}

Pipeline::~Pipeline()
{
    qDeleteAll(m_converters);
    m_converters.clear();

    if (m_parser) {
        delete m_parser;
        m_parser = nullptr;
    }
}

Pipeline &Pipeline::operator<<(AbstractConverter *converter)
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

    for (AbstractConverter *converter : m_converters) {
        GcodeParser *parserPtr = (parser && converter->needsParser()) ? parser : nullptr;

        if (parserPtr) parserPtr->pushState();
        bool modified = converter->convertLine(item, gcode, currentIndex, parserPtr);
        if (parserPtr) parserPtr->popState();

        if (modified) wasModified = true;
    }

    return wasModified;
}

bool Pipeline::needsParser() const
{
    for (const AbstractConverter *converter : m_converters) {
        if (converter->needsParser()) return true;
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

    for (AbstractConverter *converter : m_converters) {
        converter->reset();
    }

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
        processLine(i);
        converted++;
        m_currentIndex++;

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

    // Local copy prevents dangling reference if a converter calls gcode->insert(),
    // which may cause QList to reallocate its internal buffer.
    GCodeItem item = (*m_gcode)[index];
    bool wasModified = false;

    for (AbstractConverter *converter : m_converters) {
        GcodeParser *parserPtr = converter->needsParser() ? m_parser : nullptr;

        if (parserPtr) parserPtr->pushState();
        bool modified = converter->convertLine(item, m_gcode, index, parserPtr);
        if (parserPtr) parserPtr->popState();

        if (modified) wasModified = true;
    }

    if (wasModified) {
        (*m_gcode)[index] = item;
    }

    // Must run for every line so the parser position stays correct for the next call.
    m_parser->addCommand((*m_gcode)[index]);

    return wasModified;
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
    m_parser->reset();

    // result->count() may grow when a converter inserts lines (e.g. ArcsToLines).
    // The loop condition is re-evaluated each iteration so inserted lines are processed too.
    for (int i = 0; i < result->count(); ++i) {
        GCodeItem item = (*result)[i]; // local copy - see processLine()
        bool wasModified = false;

        for (AbstractConverter *converter : m_converters) {
            GcodeParser *parserPtr = converter->needsParser() ? m_parser : nullptr;

            if (parserPtr) parserPtr->pushState();
            bool modified = converter->convertLine(item, result, i, parserPtr);
            if (parserPtr) parserPtr->popState();

            if (modified) wasModified = true;
        }

        if (wasModified) {
            (*result)[i] = item;
        }

        m_parser->addCommand((*result)[i]);
    }

    return result;
}
