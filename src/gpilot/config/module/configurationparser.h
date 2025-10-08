// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONPARSER_H
#define CONFIGURATIONPARSER_H

#include <QObject>
#include "configurationmodule.h"

class ConfigurationParser : public ConfigurationModule
{
    friend class frmSettings;

    Q_OBJECT
    Q_PROPERTY(double arcApproximationLength MEMBER m_arcApproximationLength NOTIFY changed)
    Q_PROPERTY(double arcApproximationAngle MEMBER m_arcApproximationAngle NOTIFY changed)
    // Why enum cannot be the first property?
    Q_PROPERTY(ParserArcApproximationMode arcApproximationMode MEMBER m_arcApproximationMode NOTIFY changed)

    public:
        explicit ConfigurationParser(QObject *parent);
        ConfigurationParser& operator=(const ConfigurationParser&) { return *this; }
        QString getSectionName() override { return "parser"; }

        enum ParserArcApproximationMode {
            ByLength,
            ByAngle
        };
        Q_ENUM(ParserArcApproximationMode);

        ParserArcApproximationMode arcApproximationMode() const { return m_arcApproximationMode; }
        double arcApproximationLength() const { return m_arcApproximationLength; }
        double arcApproximationAngle() const { return m_arcApproximationAngle; }
        double arcApproximationValue() const { return m_arcApproximationMode == ByLength ? m_arcApproximationLength : m_arcApproximationAngle; }

    private:
        ParserArcApproximationMode m_arcApproximationMode;
        double m_arcApproximationLength;
        double m_arcApproximationAngle;
};

#endif // CONFIGURATIONPARSER_H
