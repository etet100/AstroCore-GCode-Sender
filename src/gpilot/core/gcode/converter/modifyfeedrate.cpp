// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "modifyfeedrate.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtGlobal>
#include <cmath>

QString ModifyFeedRate::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Modify feed rate",
  "description": "Scales every F (feed rate) value in the G-code by a percentage. Comments are left untouched.",
  "image": ":/images/converters/modifyfeedrate.svg",
  "fields": [
    {
      "name": "percent",
      "label": "Feed rate",
      "type": "float",
      "min": 1.0,
      "max": 1000.0,
      "default": 100.0,
      "description": "Scaling factor applied to every F value in percent. 100 = no change, 50 = half speed, 200 = double."
    }
  ]
})JSON");
}

ModifyFeedRate::ModifyFeedRate(double percent)
    : m_percent(percent)
{
}

QList<GCodeItem> ModifyFeedRate::push(const GCodeItem &input)
{
    if (input.state == GCodeItem::EmptyLine || input.state == GCodeItem::Comment) {
        return { input };
    }
    if (qIsNaN(GcodePreprocessorUtils::parseCoord(input.args, 'F'))) {
        return { input };
    }

    // Split line so we only modify the command part, not inline comments
    int parenPos = input.line.indexOf('(');
    int semiPos  = input.line.indexOf(';');
    int splitPos = -1;
    if (parenPos >= 0 && semiPos >= 0) splitPos = qMin(parenPos, semiPos);
    else if (parenPos >= 0)            splitPos = parenPos;
    else if (semiPos >= 0)             splitPos = semiPos;

    QString cmdPart     = (splitPos >= 0) ? input.line.left(splitPos) : input.line;
    QString commentPart = (splitPos >= 0) ? input.line.mid(splitPos)  : QString();

    static const QRegularExpression re("[F]\\s*([0-9]*\\.?[0-9]+)",
                                       QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(cmdPart);
    if (!match.hasMatch()) {
        return { input };
    }

    double original = match.captured(1).toDouble();
    double newFeed  = original * m_percent / 100.0;

    // Compact format: up to 3 decimal places, trailing zeros removed
    QString newStr = QString("F%1").arg(newFeed, 0, 'f', 3);
    if (newStr.contains('.')) {
        newStr.remove(QRegularExpression("0+$"));
        newStr.remove(QRegularExpression("\\.$"));
    }

    cmdPart.replace(match.capturedStart(), match.capturedLength(), newStr);

    GCodeItem out = input;
    out.line = cmdPart + commentPart;
    out.args = GcodePreprocessorUtils::splitCommand(
        GcodePreprocessorUtils::removeComment(out.line)
    );

    return { out };
}
