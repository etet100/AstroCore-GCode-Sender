// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "stripcomments.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

QString StripComments::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Strip comments",
  "description": "Removes all G-code comments — both parenthetical (comment) and semicolon style ; comment. Lines that contained only a comment become empty lines.",
  "image": ":/images/converters/stripcomments.svg",
  "fields": []
})JSON");
}

StripComments::StripComments() : AbstractConverter() {}

bool StripComments::convertLine(GCodeItem &item, GCode *gcode, int currentIndex, GcodeParser *parser)
{
    Q_UNUSED(gcode); Q_UNUSED(currentIndex); Q_UNUSED(parser);

    if (item.line.isEmpty()) return false;

    static const QRegularExpression parenComment(R"(\([^)]*\))");
    static const QRegularExpression semiComment(R"(;.*)");

    QString stripped = item.line;
    stripped.remove(parenComment);
    stripped.remove(semiComment);
    stripped = stripped.trimmed();

    if (stripped == item.line.trimmed()) return false;

    item.line = stripped;
    item.comment.clear();

    if (stripped.isEmpty()) {
        item.state = GCodeItem::EmptyLine;
        item.args.clear();
    } else {
        item.args = GcodePreprocessorUtils::splitCommand(stripped);
    }

    return true;
}
