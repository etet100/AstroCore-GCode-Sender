// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "stripcomments.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"
#include <QRegularExpression>

QString StripComments::parameterSchema()
{
    return QStringLiteral(R"JSON({
  "title": "Strip comments",
  "description": "Removes all G-code comments — both parenthetical (comment) and semicolon style ; comment. Lines that contained only a comment are dropped from the stream.",
  "image": ":/images/converters/stripcomments.png",
  "fields": []
})JSON");
}

QList<GCodeItem> StripComments::push(const GCodeItem &input)
{
    static const QRegularExpression parenComment(R"(\([^)]*\))");
    static const QRegularExpression semiComment(R"(;.*)");

    if (input.line.isEmpty()) {
        return {};
    }

    QString stripped = input.line;
    stripped.remove(parenComment);
    stripped.remove(semiComment);
    stripped = stripped.trimmed();

    if (stripped.isEmpty()) {
        return {};
    }

    if (stripped == input.line.trimmed()) {
        return { input };
    }

    GCodeItem modified = input;
    modified.line = stripped;
    modified.comment.clear();
    modified.args = GcodePreprocessorUtils::splitCommand(stripped);

    return { modified };
}
