// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeloader.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include <QDebug>
#include <QTextStream>

GCodeLoaderConfiguration GCodeLoaderConfiguration::s_current;

GCodeLoader::GCodeLoader(QObject *parent)
    : AbstractGCodeLoader(parent)
{
}

void GCodeLoader::loadFromFile(const QString &fileName)
{
    const auto& configuration = GCodeLoaderConfiguration::current();
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[GCodeLoader] Failed to open file:" << fileName;
        emit cancelled();
        return;
    }

    qDebug() << "[GCodeLoader] Loading file:" << fileName << "size:" << file.size() << "bytes";

    emit started();
    m_cancel = false;

    const qint64 fileSize = file.size();
    const int estimatedLines = qMax(1, (int)(fileSize / 20));

    GcodeParser parser;
    parser.reservePoints(estimatedLines);

    GCode* gcode = new GCode();
    gcode->reserve(estimatedLines);

    QTextStream stream(&file);
    int lastPercentage = -1;

    int lineNumber = 1;
    while (!stream.atEnd()) {
        GCodeItem item = GcodePreprocessorUtils::parseLine(stream.readLine());
        if (item.state == GCodeItem::EmptyLine) {
            continue;
        }

        item.lineNumber = lineNumber++;
        item.commandNumber = parser.getCommandNumber();
        item.isMovement = parser.addCommand(item) != nullptr;
        *gcode << std::move(item);

        int percentage = (int)(file.pos() * 100LL / fileSize);
        if (percentage != lastPercentage) {
            lastPercentage = percentage;
            emit progress(percentage);
        }

        if (m_cancel || QThread::currentThread()->isInterruptionRequested()) {
            delete gcode;
            emit cancelled();
            return;
        }
    }

    file.close();

    qDebug() << "[GCodeLoader] GCode loaded. Total lines:" << gcode->count();

    GCodeViewParser* viewParser = new GCodeViewParser();
    viewParser->getLinesFromParser(
        &parser,
        configuration.arcApproximationValue(),
        configuration.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    );

    qDebug() << "[GCodeLoader] GCodeViewParser created. Total segments:" << viewParser->getLines().count();

    if (m_cancel) {
        delete gcode;
        emit cancelled();
    } else {
        emit progress(100);
        GCodeLoaderData *result = new GCodeLoaderData();
        result->gcode = gcode;
        result->viewParser = viewParser;
        emit finished(result);
    }
}

std::optional<GCodeLoaderData> GCodeLoader::loadFromLines(const QStringList &lines)
{
    const auto& configuration = GCodeLoaderConfiguration::current();
    qDebug() << "[GCodeLoader] Loading from" << lines.size() << "lines";
    emit started();
    m_cancel = false;

    const int size = lines.size();

    GcodeParser parser;
    parser.reservePoints(size);

    GCode* gcode = new GCode();
    gcode->reserve(size);

    int lastPercentage = -1;
    int i = 0;

    for (const QString &line : lines) {
        GCodeItem item = GcodePreprocessorUtils::parseLine(line);
        if (item.state != GCodeItem::EmptyLine) {
            item.commandNumber = parser.getCommandNumber();
            item.isMovement = parser.addCommand(item) != nullptr;
            *gcode << std::move(item);
        }

        int percentage = ++i * 100 / size;
        if (percentage != lastPercentage) {
            lastPercentage = percentage;
            emit progress(percentage);
        }

        if (m_cancel || QThread::currentThread()->isInterruptionRequested()) {
            delete gcode;
            emit cancelled();

            return std::nullopt;
        }
    }

    GCodeViewParser* viewParser = new GCodeViewParser();
    viewParser->getLinesFromParser(
        &parser,
        configuration.arcApproximationValue(),
        configuration.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    );

    if (m_cancel) {
        delete gcode;
        emit cancelled();

        return std::nullopt;
    }

    emit progress(100);
    GCodeLoaderData *resultPtr = new GCodeLoaderData();
    resultPtr->gcode = gcode;
    resultPtr->viewParser = viewParser;
    emit finished(resultPtr);

    return GCodeLoaderData{gcode, viewParser};
}

void GCodeLoader::update(GCode* gcode)
{
    const auto& configuration = GCodeLoaderConfiguration::current();
    qDebug() << "[GCodeLoader] Updating" << gcode->count() << "items";
    emit started();

    m_cancel = false;
    int size = gcode->count();
    int remaining = size;
    int lastPercentage = -1;

    GcodeParser parser;
    parser.reservePoints(size);

    for (auto& item : *gcode) {
        item.commandNumber = parser.getCommandNumber();
        item.isMovement = parser.addCommand(item) != nullptr;

        remaining--;
        int percentage = 100 - (remaining * 100 / size);
        if (percentage != lastPercentage) {
            lastPercentage = percentage;
            emit progress(percentage);
        }

        if (m_cancel || QThread::currentThread()->isInterruptionRequested()) {
            emit cancelled();
            return;
        }
    }

    GCodeViewParser* viewParser = new GCodeViewParser();
    viewParser->getLinesFromParser(
        &parser,
        configuration.arcApproximationValue(),
        configuration.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    );

    if (m_cancel) {
        emit cancelled();
    } else {
        emit progress(100);

        GCodeLoaderData *result = new GCodeLoaderData();
        result->gcode = gcode;
        result->viewParser = viewParser;

        emit finished(result);
    }
}

void GCodeLoader::cancel()
{
}
