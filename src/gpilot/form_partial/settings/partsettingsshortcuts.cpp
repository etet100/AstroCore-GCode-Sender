// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingsshortcuts.h"
#include "ui_partsettingsshortcuts.h"
#include <QStyledItemDelegate>
#include <QKeySequenceEdit>
#include <QKeyEvent>

class CustomKeySequenceEdit : public QKeySequenceEdit
{
    public:
        explicit CustomKeySequenceEdit(QWidget *parent = 0): QKeySequenceEdit(parent) {}
        ~CustomKeySequenceEdit() {}

    protected:
        void keyPressEvent(QKeyEvent *pEvent) {
            QKeySequenceEdit::keyPressEvent(pEvent);
            QString s = keySequence().toString().split(", ").first();

            QString shiftedKeys = "~!@#$%^&*()_+{}|:?><\"";
            QString key = s.right(1);

            if (pEvent->modifiers() & Qt::KeypadModifier) s = "Num+" + s;
            else if (!key.isEmpty() && shiftedKeys.contains(key)) {
                s.remove("Shift+");
                s = s.left(s.size() - 1) + QString("Shift+%1").arg(key);
            }

            QKeySequence seq(QKeySequence::fromString(s));
            setKeySequence(seq);
        }
};

class ShortcutDelegate: public QStyledItemDelegate
{
    public:
        ShortcutDelegate() {}

        QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
        {
            Q_UNUSED(option);
            Q_UNUSED(index);

            return new CustomKeySequenceEdit(parent);
        }

        void setEditorData(QWidget *editor, const QModelIndex &index) const override
        {
            static_cast<QKeySequenceEdit*>(editor)->setKeySequence(index.data(Qt::DisplayRole).toString());
        }

        void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
        {
            model->setData(index, static_cast<QKeySequenceEdit*>(editor)->keySequence().toString());
        }
};

partSettingsShortcuts::partSettingsShortcuts(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::frmSettingsShortcuts)
{
    ui->setupUi(this);

    ui->tblShortcuts->setItemDelegateForColumn(2, new ShortcutDelegate);
    ui->tblShortcuts->setTabKeyNavigation(false);
    ui->tblShortcuts->setEditTriggers(QAbstractItemView::AllEditTriggers);
}

partSettingsShortcuts::~partSettingsShortcuts()
{
    delete ui;
}

void partSettingsShortcuts::setDefaults()
{
    QTableWidget *table = ui->tblShortcuts;

    for (int i = 0; i < table->rowCount(); i++) {
        QString s = table->item(i, 0)->data(Qt::DisplayRole).toString();
        //table->item(i, 2)->setData(Qt::DisplayRole, d.keys().contains(s) ? d[s] : "");
        // @TODO: translations?
        table->item(i, 2)->setData(Qt::DisplayRole, s);
    }
}
