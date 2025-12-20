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

    QList<QAction*> actions;
    actions.append(new QAction("Dummy Action", this));
    actions.append(new QAction("Another Action", this));
    actions.append(new QAction("Sample Action", this));
    setShortcuts(actions);
}

partSettingsShortcuts::~partSettingsShortcuts()
{
    delete ui;
}

void partSettingsShortcuts::setShortcuts(QList<QAction*> acts)
{
    QTableWidget *table = ui->tblShortcuts;

    table->clear();
    table->setColumnCount(3);
    table->setRowCount(acts.count());
    table->setHorizontalHeaderLabels(QStringList() << tr("Command") << tr("Text") << tr("Shortcuts"));

    table->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    table->verticalHeader()->setFixedWidth(table->verticalHeader()->sizeHint().width() + 11);

    std::sort(acts.begin(), acts.end(), [] (QAction *a1, QAction *a2) { return a1->objectName() < a2->objectName(); });
    int i = 0;
    QTableWidgetItem* wi;
    for (auto& act : acts) {
        wi = new QTableWidgetItem(act->objectName());
        wi->setFlags(Qt::ItemIsEnabled);
        table->setItem(i, 0, wi);
        wi = new QTableWidgetItem(act->text().remove("&"));
        wi->setFlags(Qt::ItemIsEnabled);
        table->setItem(i, 1, wi);
        wi = new QTableWidgetItem(act->shortcut().toString());
        wi->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
        table->setItem(i, 2, wi);
        i++;
    }

    table->resizeColumnsToContents();
    table->setMinimumHeight(table->rowHeight(0) * 10
                            + table->horizontalHeader()->height() + table->frameWidth() * 2);
    table->horizontalHeader()->setMinimumSectionSize(table->horizontalHeader()->sectionSize(2));
    table->horizontalHeader()->setStretchLastSection(true);
}

void partSettingsShortcuts::setDefaults()
{
    QMap<QString, QString> d;
    d["actFileNew"] = "Ctrl+N";
    d["actFileOpen"] = "Ctrl+O";
    d["actFileSave"] = "Ctrl+S";
    d["actFileSaveAs"] = "Ctrl+Shift+S";
    d["actJogXPlus"] = "Num+6";
    d["actJogXMinus"] = "Num+4";
    d["actJogYPlus"] = "Num+8";
    d["actJogYMinus"] = "Num+2";
    d["actJogZPlus"] = "Num+9";
    d["actJogZMinus"] = "Num+3";
    d["actJogStop"] = "Num+5";
    d["actJogStepNext"] = "Num+1";
    d["actJogStepPrevious"] = "Num+7";
    d["actJogFeedNext"] = "Num++";
    d["actJogFeedPrevious"] = "Num+-";
    d["actJogKeyboardControl"] = "ScrollLock";
    d["actSpindleOnOff"] = "Num+0";
    d["actSpindleSpeedPlus"] = "Num+*";
    d["actSpindleSpeedMinus"] = "Num+/";

    QTableWidget *table = ui->tblShortcuts;

    for (int i = 0; i < table->rowCount(); i++) {
        QString s = table->item(i, 0)->data(Qt::DisplayRole).toString();
        table->item(i, 2)->setData(Qt::DisplayRole, d.keys().contains(s) ? d[s] : "");
        // @TODO: translations?
        table->item(i, 2)->setData(Qt::DisplayRole, s);
    }
}
