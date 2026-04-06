#ifndef MACROS_H
#define MACROS_H

#include <QList>
#include <optional>
#include "macro.h"

class Macros : public QList<Macro>
{
    public:
        using QList<Macro>::QList;

        QList<Macro> byType(MacroType type) const {
            QList<Macro> result;
            for (const auto& macro : *this) {
                if (macro.type == type && macro.enabled) {
                    result.append(macro);
                }
            }

            return result;
        }

        std::optional<Macro> firstByType(MacroType type) const {
            for (const auto& macro : *this) {
                if (macro.type == type && macro.enabled) {
                    return macro;
                }
            }

            return std::nullopt;
        }
};

#endif // MACROS_H
