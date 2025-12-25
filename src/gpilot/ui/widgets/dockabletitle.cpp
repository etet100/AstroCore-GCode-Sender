#include "dockabletitle.h"
#include "ui_dockabletitle.h"
#include "ui/utils/thememanager.h"
#include "utils/utils.h"

DockableTitle::DockableTitle(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::dockableTitle)
{
    m_dockWidgetParent = qobject_cast<QDockWidget*>(parent);
    assert(m_dockWidgetParent != nullptr);

    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground);

    ui->lblTitle->setText(parent->windowTitle());
    connect(parent, &QWidget::windowTitleChanged, this, [this](const QString& title) {
        ui->lblTitle->setText(title);
    });    
    connect(m_dockWidgetParent, &QDockWidget::topLevelChanged, this, [this](bool floating) {
        ui->btnFloating->setIcon(QIcon(QString(":/images/dockable_%1dock.png").arg(floating ? "" : "un")));
        if (m_dark) {
            Utils::invertButtonIconColors(ui->btnFloating);
        }
    });

    connect(m_dockWidgetParent, &QDockWidget::featuresChanged, this, [this]() {
        ui->btnClose->setVisible(m_dockWidgetParent->features() & QDockWidget::DockWidgetClosable);
        ui->btnFloating->setVisible(m_dockWidgetParent->features() & QDockWidget::DockWidgetFloatable);
    });

    m_dark = ThemeManager::instance().dark();
    if (m_dark) {
        Utils::invertButtonIconColors(ui->btnClose);
        Utils::invertButtonIconColors(ui->btnFloating);
    }
    this->setProperty("dark", m_dark ? "true" : "false");

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this](bool dark) {
        if (m_dark != dark) {
            m_dark = dark;
            Utils::invertButtonIconColors(ui->btnClose);
            Utils::invertButtonIconColors(ui->btnFloating);
            this->setProperty("dark", m_dark ? "true" : "false");
            Utils::refreshStyle(this);
        }
    });

    Utils::refreshStyle(this);
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
