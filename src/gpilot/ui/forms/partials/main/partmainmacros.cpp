#include "partmainmacros.h"
#include "ui_partmainmacros.h"
#include "ui/utils/flowlayout.h"
#include "xswitchbuttonwithlabel.h"

PartMainMacros::PartMainMacros(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::PartMainMacros)
{
    ui->setupUi(this);

    FlowLayout *flowLayout = new FlowLayout();
    flowLayout->setContentsMargins(0, 1, 0, 1);
    flowLayout->setSpacing(2);

    QStringList macroNames = {"B.pause", "A.pause", "B.tool change", "A.tool change", "Macro 1", "Macro 2", "Macro 3", "Macro 4", "Macro 5"};

    for (auto &name : macroNames) {
        XSwitchButtonWithLabel *btn = new XSwitchButtonWithLabel(this);
        btn->setText("<html><body>" + name + " <a href=\"a\" style=\"text-decoration: underline;\"><font size=\"-2\">(edit)</font></a></body></html>");
        connect(btn, &XSwitchButtonWithLabel::linkActivated, this, [this, name](const QString &link) {
            qDebug() << "Edit macro" << name << link;
        });
        flowLayout->addWidget(btn);
    }

    setLayout(flowLayout);
}

PartMainMacros::~PartMainMacros()
{
    delete ui;
}
