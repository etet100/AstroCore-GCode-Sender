// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "gcodeloader.h"
#include "core/gcode/parser/gcodeparser.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include <QDebug>

class StringListIODevice : public QIODevice {
    public:
        StringListIODevice(const QStringList &lines, QObject *parent = nullptr)
            : QIODevice(parent), lines(lines), index(0) {}

        bool open(OpenMode mode) override {
            index = 0;
            return QIODevice::open(mode);
        }

        bool isSequential() const override { return true; }

    protected:
        qint64 readData(char *data, qint64 maxSize) override {
            if (index >= lines.size()) return -1;
            QByteArray ba = lines[index++].toUtf8() + '\n';
            qint64 size = qMin(maxSize, (qint64)ba.size());
            memcpy(data, ba.constData(), size);
            return size;
        }

        qint64 writeData(const char*, qint64) override { return -1; }

    private:
        QStringList lines;
        int index;
};

GCodeLoader::GCodeLoader(QObject *parent)
    : AbstractGCodeLoader(parent)
{
}

void GCodeLoader::loadFromFile(const QString &fileName, GCodeLoaderConfiguration &configuration)
{
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly)) {
        emit cancelled();

        return;
    }

    loadFromIODevice(file, file.size(), configuration);
    file.close();
}

void GCodeLoader::loadFromLines(const QStringList &lines, GCodeLoaderConfiguration &configuration)
{
    StringListIODevice io(lines);

    loadFromIODevice(io, lines.size(), configuration);
}

void GCodeLoader::update(GCode* gcode, GCodeLoaderConfiguration& configuration)
{
    emit started();

    m_cancel = false;
    int size = gcode->count();
    int remaining = size;

    GcodeParser parser;
    for (auto& item : *gcode) {
        item.lineNumber = parser.getCommandNumber();
        parser.addCommand(item.args);

        remaining--;
        int percentage = 100 - (remaining * 100 / size);
        static int lastPercentage = 0;
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

void GCodeLoader::loadFromIODevice(QIODevice &io, int size, GCodeLoaderConfiguration &configuration)
{
    emit started();

    m_cancel = false;

    int remaining = size;
    GcodeParser parser;
    GCode* gcode = new GCode();

    while (!io.atEnd()) {
        GCodeItem item = GcodePreprocessorUtils::parseLine(io.readLine().toStdString());
        if (item.state == GCodeItem::EmptyLine) {
            continue;
        }

        item.lineNumber = parser.getCommandNumber();
        parser.addCommand(item.args);
        *gcode << item;

        remaining = size - io.pos();

        int percentage = 100 - (remaining * 100 / size);
        static int lastPercentage = 0;
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

    GCodeViewParser* viewParser = new GCodeViewParser();
    viewParser->getLinesFromParser(
        &parser,
        configuration.arcApproximationValue(),
        configuration.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    );

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

void GCodeLoader::cancel()
{
}
