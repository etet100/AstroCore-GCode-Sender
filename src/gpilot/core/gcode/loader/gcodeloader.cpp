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

void GCodeLoader::loadFromIODevice(QIODevice &io, int size, GCodeLoaderConfiguration &configuration)
{
    emit started();

    qDebug() << configuration.arcApproximationValue();
    qDebug() << configuration.arcApproximationMode();

    m_cancel = false;

    std::string command;
    std::string stripped;
    std::string trimmed;
    std::string comment;
    QList<QString> args;
    int remaining = size;
    GcodeParser parser;
    GCode* gcode = new GCode();
    PointSegment* ps = nullptr;

    while (!io.atEnd()) {
        command = io.readLine().toStdString();

        trimmed = GcodePreprocessorUtils::trimCommand(command);

        if (!trimmed.empty()) {
            // Split command
            stripped = GcodePreprocessorUtils::removeComment(command);
            args = GcodePreprocessorUtils::splitCommand(stripped);
            comment = GcodePreprocessorUtils::getComment(command);
            if (stripped.empty() && comment.empty()) {
                break;
            }

            GCodeItem item;
            item.command = QString::fromStdString(stripped);
            item.comment = QString::fromStdString(GcodePreprocessorUtils::getComment(command));
            item.state = GCodeItem::InQueue;
            item.lineNumber = parser.getCommandNumber();
            item.args = args;
            item.group = GCodeItemGroup::Unknown; // TODO: determine group
            if (stripped.empty()) {
                item.state = GCodeItem::Comment;
                item.group = GCodeItemGroup::Comment;
            }
            item.ps = parser.addCommand(item);

            *gcode << item;
        }

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
    // viewParser->getLinesFromGCode(
    //     *gcode,
    //     configuration.arcApproximationValue(),
    //     configuration.arcApproximationMode() == ConfigurationParser::ParserArcApproximationMode::ByAngle
    // );

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
