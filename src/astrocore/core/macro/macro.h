#ifndef MACRO_H
#define MACRO_H

#include <QString>

enum class MacroType {
    ProgramStart,
    ProgramEnd,
    BeforePause,
    AfterPause,
    BeforeToolChange,
    AfterToolChange,
    Custom,
};

struct Macro {
    bool enabled = true;
    QString name;
    QString content;
    MacroType type = MacroType::Custom;
};

#endif // MACRO_H
