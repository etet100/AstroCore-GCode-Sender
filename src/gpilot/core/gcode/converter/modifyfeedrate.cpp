// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "modifyfeedrate.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>
#include <QtGlobal>
#include <cmath>

ModifyFeedRate::ModifyFeedRate(double percent)
    : AbstractConverter()
    , m_percent(percent)
{
}

bool ModifyFeedRate::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(gcode); Q_UNUSED(currentIndex); Q_UNUSED(parser);

    if (item.state == GCodeItem::EmptyLine || item.state == GCodeItem::Comment) return false;
    if (qIsNaN(GcodePreprocessorUtils::parseCoord(item.args, 'F'))) return false;

    // Split line so we only modify the command part, not inline comments
    int parenPos = item.line.indexOf('(');
    int semiPos  = item.line.indexOf(';');
    int splitPos = -1;
    if (parenPos >= 0 && semiPos >= 0) splitPos = qMin(parenPos, semiPos);
    else if (parenPos >= 0)            splitPos = parenPos;
    else if (semiPos >= 0)             splitPos = semiPos;

    QString cmdPart     = (splitPos >= 0) ? item.line.left(splitPos) : item.line;
    QString commentPart = (splitPos >= 0) ? item.line.mid(splitPos)  : QString();

    static const QRegularExpression re("[F]\\s*([0-9]*\\.?[0-9]+)",
                                       QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatch match = re.match(cmdPart);
    if (!match.hasMatch()) return false;

    double original = match.captured(1).toDouble();
    double newFeed  = original * m_percent / 100.0;

    // Compact format: up to 3 decimal places, trailing zeros removed
    QString newStr = QString("F%1").arg(newFeed, 0, 'f', 3);
    if (newStr.contains('.')) {
        newStr.remove(QRegularExpression("0+$"));
        newStr.remove(QRegularExpression("\\.$"));
    }

    cmdPart.replace(match.capturedStart(), match.capturedLength(), newStr);

    item.line = cmdPart + commentPart;
    item.args = GcodePreprocessorUtils::splitCommand(
        GcodePreprocessorUtils::removeComment(item.line)
    );
    return true;
}
