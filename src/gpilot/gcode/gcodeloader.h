// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef GCODELOADER_H
#define GCODELOADER_H

#include "gcode.h"
#include "parser/gcodeviewparser.h"
#include "config/module/configurationparser.h"
#include <QFile>
#include <QThread>

struct GCodeLoaderData {
    GCode *gcode;
    GCodeViewParser *viewParser;
};

class GCodeLoaderConfiguration {
    public:
        GCodeLoaderConfiguration(ConfigurationParser &configuration) {
            m_arcApproximationMode = configuration.arcApproximationMode();
            m_arcApproximationValue = configuration.arcApproximationValue();
        }
        double arcApproximationValue() const { return m_arcApproximationValue; }
        ConfigurationParser::ParserArcApproximationMode arcApproximationMode() const { return m_arcApproximationMode; }
    private:
        ConfigurationParser::ParserArcApproximationMode m_arcApproximationMode;
        double m_arcApproximationValue;
};

class AbstractGCodeLoader : public QObject
{
    Q_OBJECT

    public:
        explicit AbstractGCodeLoader(QObject *parent = nullptr) : QObject(parent) {}
        virtual void loadFromFile(const QString &fileName, GCodeLoaderConfiguration &configuration) = 0;
        virtual void loadFromLines(const QStringList &lines, GCodeLoaderConfiguration &configuration) = 0;
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
        explicit GCodeLoader(QObject *parent = nullptr);
        void loadFromFile(const QString &fileName, GCodeLoaderConfiguration &configuration) override;
        void loadFromLines(const QStringList &lines, GCodeLoaderConfiguration &configuration) override;
        void cancel() override;

    private:
    //    static const int PROGRESSSTEP = 1000;
        bool m_cancel;
        void loadFromFileObject(QFile &stream, int size, GCodeLoaderConfiguration &configuration);
};

#endif // GCODELOADER_H
