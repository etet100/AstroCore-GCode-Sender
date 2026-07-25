#include "macros.h"

Macros::Macros(QObject *parent)
    : QObject(parent)
{
}

void Macros::append(const Macro& macro)
{
    m_list.append(macro);
    notifyUpdated();
}

void Macros::clear()
{
    if (m_list.isEmpty()) {
        return;
    }
    m_list.clear();
    notifyUpdated();
}

void Macros::remove(int index)
{
    if (index < 0 || index >= m_list.size()) {
        return;
    }

    m_list.removeAt(index);
    notifyUpdated();
}

QList<Macro> Macros::byType(MacroType type) const
{
    QList<Macro> result;
    for (const auto& macro : m_list) {
        if (macro.type == type && macro.enabled) {
            result.append(macro);
        }
    }

    return result;
}

std::optional<Macro> Macros::firstByType(MacroType type) const
{
    for (const auto& macro : m_list) {
        if (macro.type == type && macro.enabled) {
            return macro;
        }
    }

    return std::nullopt;
}

void Macros::beginUpdate()
{
    m_multUpdates = true;
}

void Macros::endUpdate()
{
    m_multUpdates = false;
    notifyUpdated();
}

void Macros::notifyUpdated()
{
    if (!m_multUpdates) {
        emit updated();
    }
}
