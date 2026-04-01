// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#pragma once

#include <QVector3D>
#include <QStringList>
#include <optional>

struct ProbeResult {
    QVector3D position;
    bool contacted = false;
};

// Parses [PRB:x,y,z:contact] from a multi-line response.
class ProbeResponseParser
{
    public:
        static std::optional<ProbeResult> parse(const QStringList &lines);
};
