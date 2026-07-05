#include "macrocompiler.h"
#include "core/gcode/gcode.h"
#include "core/gcode/parser/gcodepreprocessorutils.h"

GCode *MacroCompiler::build(const Macro &macro)
{
    GCode *gcode = build(macro.content);
    gcode->setName(macro.name);
    gcode->setType(GCodeType::Macro);
    return gcode;
}

GCode *MacroCompiler::build(const QString &content)
{
    auto *gcode = new GCode();
    QStringList lines = content.split('\n');
    GcodePreprocessorUtils::parseLines(lines, *gcode);

    return gcode;
}
