// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "shortcutsmanager.h"

// --- ShortcutNode ---

ShortcutNode::ShortcutNode(const QString &name)
    : m_name(name)
{
}

ShortcutNode::~ShortcutNode()
{
    qDeleteAll(m_children);
}

QString ShortcutNode::name() const
{
    return m_name;
}

const QList<ShortcutNode *> &ShortcutNode::children() const
{
    return m_children;
}

const QList<QAction *> &ShortcutNode::actions() const
{
    return m_actions;
}

ShortcutNode *ShortcutNode::findOrCreateChild(const QString &name)
{
    for (auto *child : m_children) {
        if (child->name() == name) {
            return child;
        }
    }

    auto *node = new ShortcutNode(name);
    m_children.append(node);

    return node;
}

void ShortcutNode::addAction(QAction *action)
{
    m_actions.append(action);
}

// --- ShortcutsManager ---

ShortcutsManager &ShortcutsManager::instance()
{
    static ShortcutsManager manager;

    return manager;
}

ShortcutsManager::ShortcutsManager(QObject *parent)
    : QObject(parent)
{
}

ShortcutsManager::~ShortcutsManager()
{
    qDeleteAll(m_roots);
}

void ShortcutsManager::registerAction(const QString &path, QAction *action)
{
    QStringList parts = path.split('/', Qt::SkipEmptyParts);
    if (parts.isEmpty()) {
        return;
    }

    ShortcutNode *node = findOrCreateRoot(parts.first());
    for (int i = 1; i < parts.size(); ++i) {
        node = node->findOrCreateChild(parts.at(i));
    }

    node->addAction(action);
}

const QList<ShortcutNode *> &ShortcutsManager::rootNodes() const
{
    return m_roots;
}

QList<QAction *> ShortcutsManager::allActions() const
{
    QList<QAction *> result;
    for (auto *root : m_roots) {
        collectActions(root, result);
    }

    return result;
}

QList<ShortcutEntry> ShortcutsManager::exportList() const
{
    QList<ShortcutEntry> result;
    for (QAction *action : allActions()) {
        QStringList sequences;
        for (const QKeySequence &seq : action->shortcuts()) {
            sequences.append(seq.toString());
        }
        result.append({action->objectName(), sequences});
    }

    return result;
}

void ShortcutsManager::importList(const QList<ShortcutEntry> &list)
{
    for (const ShortcutEntry &entry : list) {
        QAction *action = findAction(entry.objectName);
        if (action) {
            QList<QKeySequence> sequences;
            for (const QString &s : entry.keySequences) {
                sequences.append(QKeySequence::fromString(s));
            }
            action->setShortcuts(sequences);
        }
    }
}

QAction *ShortcutsManager::findAction(const QString &objectName) const
{
    for (auto *action : allActions()) {
        if (action->objectName() == objectName) {
            return action;
        }
    }

    return nullptr;
}

ShortcutNode *ShortcutsManager::findOrCreateRoot(const QString &name)
{
    for (auto *root : m_roots) {
        if (root->name() == name) {
            return root;
        }
    }

    auto *node = new ShortcutNode(name);
    m_roots.append(node);

    return node;
}

void ShortcutsManager::collectActions(const ShortcutNode *node, QList<QAction *> &out) const
{
    out.append(node->actions());
    for (auto *child : node->children()) {
        collectActions(child, out);
    }
}
