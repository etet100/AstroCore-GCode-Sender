// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsshortcuts.h"
#include "ui_partsettingsshortcuts.h"
#include "ui/utils/shortcutsmanager.h"
#include "ui/config/configurationui.h"
#include <QStyledItemDelegate>
#include <QKeySequenceEdit>
#include <QKeyEvent>
#include <QHeaderView>

class CustomKeySequenceEdit : public QKeySequenceEdit
{
public:
    explicit CustomKeySequenceEdit(QWidget *parent = nullptr) : QKeySequenceEdit(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override
    {
        QKeySequenceEdit::keyPressEvent(event);
        QString s = keySequence().toString().split(", ").first();

        QString shiftedKeys = "~!@#$%^&*()_+{}|:?><\"";
        QString key = s.right(1);

        if (event->modifiers() & Qt::KeypadModifier) {
            s = "Num+" + s;
        } else if (!key.isEmpty() && shiftedKeys.contains(key)) {
            s.remove("Shift+");
            s = s.left(s.size() - 1) + QString("Shift+%1").arg(key);
        }

        setKeySequence(QKeySequence::fromString(s));
    }
};

class ShortcutDelegate : public QStyledItemDelegate
{
public:
    ShortcutDelegate() {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        return new CustomKeySequenceEdit(parent);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        static_cast<QKeySequenceEdit *>(editor)->setKeySequence(index.data(Qt::DisplayRole).toString());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        model->setData(index, static_cast<QKeySequenceEdit *>(editor)->keySequence().toString());
    }
};

PartSettingsShortcuts::PartSettingsShortcuts(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::frmSettingsShortcuts)
{
    ui->setupUi(this);

    ui->tblShortcuts->setItemDelegateForColumn(2, new ShortcutDelegate);
    ui->tblShortcuts->setTabKeyNavigation(false);
    ui->tblShortcuts->setEditTriggers(QAbstractItemView::AllEditTriggers);
}

PartSettingsShortcuts::~PartSettingsShortcuts()
{
    delete ui;
}

void PartSettingsShortcuts::populate()
{
    QTableWidget *table = ui->tblShortcuts;
    table->clearContents();
    table->setRowCount(0);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({tr("Command"), tr("Text"), tr("Shortcut")});

    for (const ShortcutNode *node : ShortcutsManager::instance().rootNodes()) {
        addNodeRows(node, 0);
    }

    table->resizeColumnsToContents();
    table->horizontalHeader()->setStretchLastSection(true);
}

void PartSettingsShortcuts::addNodeRows(const ShortcutNode *node, int depth)
{
    QTableWidget *table = ui->tblShortcuts;

    // Category header row
    int row = table->rowCount();
    table->insertRow(row);

    QString indent = QString("  ").repeated(depth);
    auto *header = new QTableWidgetItem(indent + node->name());
    header->setFlags(Qt::ItemIsEnabled);

    QFont f = header->font();
    f.setBold(true);
    header->setFont(f);
    header->setForeground(table->palette().color(QPalette::Disabled, QPalette::Text));

    table->setItem(row, 0, header);
    table->setSpan(row, 0, 1, 3);

    // Action rows
    for (QAction *action : node->actions()) {
        row = table->rowCount();
        table->insertRow(row);

        auto *cmdItem = new QTableWidgetItem(action->objectName());
        cmdItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        cmdItem->setData(Qt::UserRole, QVariant::fromValue(action)); // used by applyChanges()
        table->setItem(row, 0, cmdItem);

        auto *textItem = new QTableWidgetItem(action->text().remove("&"));
        textItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        table->setItem(row, 1, textItem);

        auto *shortcutItem = new QTableWidgetItem(action->shortcut().toString());
        shortcutItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
        table->setItem(row, 2, shortcutItem);
    }

    for (const ShortcutNode *child : node->children()) {
        addNodeRows(child, depth + 1);
    }
}

void PartSettingsShortcuts::applyChanges()
{
    QTableWidget *table = ui->tblShortcuts;

    for (int i = 0; i < table->rowCount(); ++i) {
        QTableWidgetItem *cmdItem = table->item(i, 0);
        QTableWidgetItem *shortcutItem = table->item(i, 2);
        if (!cmdItem || !shortcutItem) {
            continue;
        }

        auto *action = qvariant_cast<QAction *>(cmdItem->data(Qt::UserRole));
        if (!action) {
            continue; // category header row
        }

        action->setShortcut(QKeySequence::fromString(shortcutItem->data(Qt::DisplayRole).toString()));
    }
}

void PartSettingsShortcuts::setDefaults()
{
    QMap<QString, QString> defaults;
    for (const ShortcutEntry &entry : ConfigurationUI::defaultShortcuts()) {
        defaults[entry.objectName] = entry.keySequences.value(0);
    }

    QTableWidget *table = ui->tblShortcuts;

    for (int i = 0; i < table->rowCount(); ++i) {
        QTableWidgetItem *cmdItem = table->item(i, 0);
        QTableWidgetItem *shortcutItem = table->item(i, 2);
        if (!cmdItem || !shortcutItem) {
            continue;
        }

        if (!qvariant_cast<QAction *>(cmdItem->data(Qt::UserRole))) {
            continue; // category header row
        }

        QString name = cmdItem->data(Qt::DisplayRole).toString();
        shortcutItem->setData(Qt::DisplayRole, defaults.value(name, ""));
    }
}
