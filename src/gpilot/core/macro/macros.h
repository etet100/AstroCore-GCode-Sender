#ifndef MACROS_H
#define MACROS_H

#include <QObject>
#include <QList>
#include <optional>
#include "macro.h"

class Macros : public QObject
{
    Q_OBJECT

public:
    explicit Macros(QObject *parent = nullptr);

    void append(const Macro& macro);
    void clear();
    void remove(int id);

    int size() const { return m_list.size(); }
    bool isEmpty() const { return m_list.isEmpty(); }
    Macro& at(int i) { return m_list[i]; }
    const Macro& at(int i) const { return m_list.at(i); }

    QList<Macro>::const_iterator begin() const { return m_list.cbegin(); }
    QList<Macro>::const_iterator end() const { return m_list.cend(); }

    QList<Macro> byType(MacroType type) const;
    std::optional<Macro> firstByType(MacroType type) const;

    // Suppress signal until endUpdate() is called
    void beginUpdate();
    void endUpdate();

signals:
    void updated();

private:
    void notifyUpdated();

    QList<Macro> m_list;
    bool m_multUpdates = false;
};

#endif // MACROS_H
