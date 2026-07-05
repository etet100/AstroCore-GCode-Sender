#include "partmainmacros.h"
#include "ui_partmainmacros.h"
// #include "xswitchbuttonwithlabel.h"
#include "ui/widgets/macrowidget.h"
#include <QToolButton>

PartMainMacros::PartMainMacros(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PartMainMacros)
{
    ui->setupUi(this);

    m_flowLayout = new FlowLayout();
    m_flowLayout->setContentsMargins(0, 1, 0, 1);
    m_flowLayout->setSpacing(2);

    setLayout(m_flowLayout);
}

PartMainMacros::~PartMainMacros()
{
    delete ui;
}

void PartMainMacros::updateMacros(const Macros& macros)
{
    while (m_flowLayout->count() > 0) {
        QLayoutItem *item = m_flowLayout->takeAt(0);
        delete item->widget();
        delete item;
    }

    for (int i = 0; i < macros.size(); ++i) {
        MacroWidget *btn = new MacroWidget(this);
        btn->setId(i);
        btn->setName(macros.at(i).name);
        connect(btn, &MacroWidget::runClicked, this, &PartMainMacros::runMacro);
        connect(btn, &MacroWidget::editClicked, this, &PartMainMacros::editMacro);
        m_flowLayout->addWidget(btn);
    }

    QToolButton *btnNew = new QToolButton(this);
    btnNew->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    btnNew->setText(tr("Add"));
    btnNew->setIcon(QIcon(":/images/settings.svg"));
    btnNew->setIconSize(QSize(16, 16));
    connect(btnNew, &QToolButton::clicked, this, &PartMainMacros::newMacroRequested);
    m_flowLayout->addWidget(btnNew);
}
