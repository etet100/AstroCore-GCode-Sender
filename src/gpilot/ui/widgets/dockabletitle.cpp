#include "dockabletitle.h"
#include "ui_dockabletitle.h"
#include <QDockWidget>

DockableTitle::DockableTitle(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::dockableTitle)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground);

    ui->lblTitle->setText(parent->windowTitle());
    connect(parent, &QWidget::windowTitleChanged, this, [this](const QString& title) {
        ui->lblTitle->setText(title);
    });
}

DockableTitle::~DockableTitle()
{
    delete ui;
}

void DockableTitle::closeClicked()
{
    QDockWidget* dockWidget = qobject_cast<QDockWidget*>(parentWidget());
    if (dockWidget) {
        dockWidget->close();
    }
}

void DockableTitle::floatingClicked()
{
    QDockWidget* dockWidget = qobject_cast<QDockWidget*>(parentWidget());
    if (dockWidget) {
        dockWidget->setFloating(!dockWidget->isFloating());
    }
}
