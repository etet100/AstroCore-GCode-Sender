// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef SHORTCUTSMANAGER_H
#define SHORTCUTSMANAGER_H

#include <QObject>
#include <QAction>
#include <QKeySequence>
#include <QList>
#include <QString>
#include "core/config/module/configurationui.h"

// One node in the shortcuts tree - a category or subcategory.
// Each node can have child nodes and a list of actions (shortcuts).
class ShortcutNode
{
public:
    explicit ShortcutNode(const QString &name);
    ~ShortcutNode();

    QString name() const;
    const QList<ShortcutNode *> &children() const;
    const QList<QAction *> &actions() const;

    // Returns an existing child with this name, or creates a new one.
    ShortcutNode *findOrCreateChild(const QString &name);
    void addAction(QAction *action);

private:
    QString m_name;
    QList<ShortcutNode *> m_children; // owned
    QList<QAction *> m_actions;       // not owned
};

// Singleton that stores all keyboard shortcuts organized in a category tree.
//
// Usage:
//   auto &mgr = ShortcutsManager::instance();
//   mgr.registerAction("File", actFileOpen);
//   mgr.registerAction("Jogging/X Axis", actJogXPlus);
//
// The tree can be read back via rootNodes() for display in a shortcuts editor,
// or as a flat list via allActions() for the existing settings table.
class ShortcutsManager : public QObject
{
    Q_OBJECT

public:
    static ShortcutsManager &instance();

    // Register an action at the given category path.
    // Use "/" to separate categories, e.g. "File" or "Jogging/X Axis".
    void registerAction(const QString &path, QAction *action);

    // Top-level category nodes.
    const QList<ShortcutNode *> &rootNodes() const;

    // Flat list of all registered actions (useful for the shortcuts settings table).
    QList<QAction *> allActions() const;

    // Find a registered action by its objectName. Returns nullptr if not found.
    QAction *findAction(const QString &objectName) const;

    // Build a flat list of all shortcuts for saving to configuration.
    QList<ShortcutEntry> exportList() const;

    // Apply shortcuts from a previously saved list to the registered actions.
    void importList(const QList<ShortcutEntry> &list);

private:
    explicit ShortcutsManager(QObject *parent = nullptr);
    ~ShortcutsManager();

    ShortcutsManager(const ShortcutsManager &) = delete;
    ShortcutsManager &operator=(const ShortcutsManager &) = delete;

    ShortcutNode *findOrCreateRoot(const QString &name);
    void collectActions(const ShortcutNode *node, QList<QAction *> &out) const;

    QList<ShortcutNode *> m_roots; // owned
};

#endif // SHORTCUTSMANAGER_H
