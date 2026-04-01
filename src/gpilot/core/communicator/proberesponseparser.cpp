// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2024 BTS

#include "proberesponseparser.h"
#include <QRegularExpression>
#include <QDebug>

std::optional<ProbeResult> ProbeResponseParser::parse(const QStringList &lines)
{
    // [PRB:x,y,z:contact]  e.g. [PRB:0.000,0.000,-8.530:1]
    static QRegularExpression re(R"(\[PRB:([\-\d\.]+),([\-\d\.]+),([\-\d\.]+):(\d)\])");

    for (const QString &line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        ProbeResult result;
        result.position.setX(match.captured(1).toDouble());
        result.position.setY(match.captured(2).toDouble());
        result.position.setZ(match.captured(3).toDouble());
        result.contacted = (match.captured(4) == "1");

        qDebug() << "[ProbeResponseParser] X=" << result.position.x()
                 << "Y=" << result.position.y()
                 << "Z=" << result.position.z()
                 << "contacted=" << result.contacted;

        return result;
    }

    qDebug() << "[ProbeResponseParser] No PRB line found in response:" << lines;

    return std::nullopt;
}
