// This file is a part of "AstroCore" application.
// Copyright 2024-2026 BTS

#include <QAction>
#include <QDebug>
#include <QDockWidget>
#include <QLayout>
#include <QSignalBlocker>
#include "ui/forms/frmmain.h"
#include "ui/config/uiconfigs.h"
#include "ui_frmmain.h"

void FrmMain::initializeCentralWidgets()
{
    m_centralWidgets = {
        {ui->program, ui->dockProgram, ui->actViewCentralProgram, "program", "G-code program"},
        {ui->visualizer, ui->dockVisualizer, ui->actViewCentralVisualizer, "visualizer", "Visualizer"}
    };
}

void FrmMain::centralWidgetActionTriggered(bool checked)
{
    QAction* action = qobject_cast<QAction*>(sender());

    // If action is being unchecked, re-check it and return
    if (!checked) {
        const QSignalBlocker blocker(action);
        action->setChecked(true);
        return;
    }

    for (auto& config : m_centralWidgets) {
        if (config.action == action) {
            switchCentralWidget(&config);
            break;
        }
    }
}

void FrmMain::switchCentralWidget(CentralWidgetConfig* requestedConfig)
{
    CentralWidgetConfig* currentConfig = nullptr;
    for (auto& config : m_centralWidgets) {
        if (config.widget->parentWidget() == ui->centralWidget) {
            currentConfig = &config;
            break;
        }
    }

    if (!currentConfig || currentConfig == requestedConfig) {
        if (requestedConfig->dock->isVisible()) {
            qWarning() << "[FrmMain] Central widget dock is visible";
        }
        requestedConfig->dock->setProperty("cw", true);

        return;
    }

    // Uncheck all other actions
    for (auto& config : m_centralWidgets) {
        if (config.name != requestedConfig->name) {
            const QSignalBlocker blocker(config.action);
            config.action->setChecked(false);
            config.dock->setProperty("cw", false);
        }
    }

    bool dockWasVisible = requestedConfig->dock->isVisible();

    // Undock requested widget
    requestedConfig->widget->setParent(nullptr);
    requestedConfig->dock->hide();

    // Remove current widget from central
    ui->centralWidget->layout()->removeWidget(currentConfig->widget);

    // Dock current widget
    currentConfig->dock->setWidget(currentConfig->widget);
    currentConfig->dock->setVisible(dockWasVisible);

    // Add requested widget to central
    ui->centralWidget->layout()->addWidget(requestedConfig->widget);
    ui->centralWidgetTitle->setTitle(requestedConfig->title);

    UiConfigs::instance().ui().setCentralWidget(requestedConfig->name);
    const QSignalBlocker blocker(requestedConfig->action);
    requestedConfig->action->setChecked(true);
    requestedConfig->dock->setProperty("cw", true);
}

void FrmMain::restoreCentralWidget()
{
    QString centralWidgetName = UiConfigs::instance().ui().centralWidget();
    if (centralWidgetName.isEmpty()) {
        // it should never be empty since it has a default value
        return;
    }

    for (auto& config : m_centralWidgets) {
        if (config.name == centralWidgetName) {
            switchCentralWidget(&config);
            break;
        }
    }
}
