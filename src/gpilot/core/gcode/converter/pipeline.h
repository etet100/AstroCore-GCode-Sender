// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PIPELINE_H
#define PIPELINE_H

#include "abstractconverter.h"
#include "core/gcode/parser/gcodeparser.h"
#include <QObject>

/**
 * Pipeline processes G-Code through a chain of converters.
 * Implements AbstractConverter interface - can be used anywhere a single AbstractConverter is expected.
 * Also provides additional helper methods for batch processing.
 */
class Pipeline : public QObject, public AbstractConverter
{
    Q_OBJECT

    public:
        explicit Pipeline(QObject *parent = nullptr);
        ~Pipeline();

        Pipeline &operator<<(AbstractConverter *converter);

        bool convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser) override;
        void reset() override;
        bool needsParser() const override;

        void setGCode(GCode *gcode);
        int convertNext(int count);
        bool hasMore() const;
        int currentPosition() const { return m_currentIndex; }
        int totalLines() const;
        GCode* convertAll();

    signals:
        void progressChanged(int current, int total);

    private:
        QList<AbstractConverter*> m_converters;
        GCode *m_gcode;
        GcodeParser *m_parser;
        int m_currentIndex;

        bool processLine(int index);
};

#endif // PIPELINE_H
