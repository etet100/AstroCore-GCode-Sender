// This file is a part of "AstroCore" application.
// Copyright 2024-2026 BTS

#include <QShortcut>
#include "ui/forms/frmmain.h"
#include "ui/utils/thememanager.h"
#include "ui/config/uiconfigs.h"
#include "ui_frmmain.h"

void FrmMain::initializeUiScaleMenu()
{
    QAction* action;
    double scale = UiConfigs::instance().ui().uiScale();
    for (int i = 80; i <= 140; i+=10) {
        action = ui->menuUIScale->addAction(QString::number(i) + "%" + (i == 100 ? " (default)" : ""));
        action->setProperty("scale", i);
        action->setCheckable(true);
        action->setChecked(i == scale);
        connect(action, &QAction::triggered, this, [this](bool checked) {
            QAction* act = qobject_cast<QAction*>(sender());
            if (checked) {
                for (auto action : ui->menuUIScale->actions()) {
                    if (action != sender()) {
                        action->setChecked(false);
                    }
                }
            } else {
                // ignore unsetting
                act->setChecked(true);
                return;
            }

            ThemeManager::instance().setScale(act->property("scale").toInt());
        });
    }

    updateUiScaleMenu();

    QShortcut* shortcutUIScaleUp = new QShortcut(QKeySequence("Ctrl++"), this);
    connect(shortcutUIScaleUp, &QShortcut::activated, this, &FrmMain::increaseUiScale);
    QShortcut* shortcutUpScaleDown = new QShortcut(QKeySequence("Ctrl+-"), this);
    connect(shortcutUpScaleDown, &QShortcut::activated, this, &FrmMain::decreaseUiScale);
    QShortcut* shortcutReset = new QShortcut(QKeySequence("Ctrl+0"), this);
    connect(shortcutReset, &QShortcut::activated, this, &FrmMain::resetUiScale);

    connect(&ThemeManager::instance(), &ThemeManager::scaleChanged, this, [this](int scale, float scaleF){
        UiConfigs::instance().ui().setUiScale(scale);
        updateUiScaleMenu();
    });
}

void FrmMain::updateUiScaleMenu()
{
    double scale = UiConfigs::instance().ui().uiScale();

    for (auto& action : ui->menuUIScale->actions()) {
        action->setChecked(action->property("scale").toInt() == scale);
    }
}

void FrmMain::decreaseUiScale()
{
    ThemeManager::instance().decreaseScale();
}

void FrmMain::resetUiScale()
{
    ThemeManager::instance().resetScale();
}

void FrmMain::increaseUiScale()
{
    ThemeManager::instance().increaseScale();
}
