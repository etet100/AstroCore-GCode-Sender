// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef ABSTRACTGCODELOADER_H
#define ABSTRACTGCODELOADER_H

#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include "core/config/module/configurationparser.h"
#include <QFile>
#include <QThread>
#include <optional>

struct GCodeLoaderData {
    GCode *gcode;
    GCodeViewParser *viewParser;
};

class GCodeLoaderConfiguration {
    public:
        static void setCurrent(ConfigurationParser &configuration) {
            s_current.m_arcApproximationMode = configuration.arcApproximationMode();
            s_current.m_arcApproximationValue = configuration.arcApproximationValue();
        }
        static const GCodeLoaderConfiguration& current() { return s_current; }

        double arcApproximationValue() const { return m_arcApproximationValue; }
        ConfigurationParser::ParserArcApproximationMode arcApproximationMode() const { return m_arcApproximationMode; }

    private:
        GCodeLoaderConfiguration() = default;
        static GCodeLoaderConfiguration s_current;

        ConfigurationParser::ParserArcApproximationMode m_arcApproximationMode = ConfigurationParser::ParserArcApproximationMode::ByAngle;
        double m_arcApproximationValue = 0;
};

class AbstractGCodeLoader : public QObject
{
    Q_OBJECT

    public:
        explicit AbstractGCodeLoader(QObject* parent = nullptr)
            : QObject(parent) {}
        virtual void loadFromFile(const QString& fileName) = 0;
        virtual std::optional<GCodeLoaderData> loadFromLines(const QStringList& lines) = 0;
        virtual void update(GCode* gcode) = 0;
        virtual void cancel() = 0;

    signals:
        void progress(int value);
        void started();
        void finished(GCodeLoaderData *result);
        void cancelled();
};

class GCodeLoader : public AbstractGCodeLoader
{
    public:
        explicit GCodeLoader(QObject* parent = nullptr);
        void loadFromFile(const QString& fileName) override;
        std::optional<GCodeLoaderData> loadFromLines(const QStringList& lines) override;
        void update(GCode* gcode) override;
        void cancel() override;

    private:
        bool m_cancel;
        void loadFromIODevice(QIODevice& io, int size);
};

#endif // ABSTRACTGCODELOADER_H
