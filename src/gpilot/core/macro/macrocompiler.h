#ifndef MACROCOMPILER_H
#define MACROCOMPILER_H

#include <QString>
#include "macro.h"

class GCode;

class MacroCompiler
{
    public:
        // Caller takes ownership of the returned GCode.
        static GCode *build(const Macro &macro);
        static GCode *build(const QString &content);
};

#endif // MACROCOMPILER_H
