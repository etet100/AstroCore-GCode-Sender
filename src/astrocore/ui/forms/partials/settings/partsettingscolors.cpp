// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#include "partsettingscolors.h"
#include "ui_partsettingscolors.h"
#include "ui/utils/thememanager.h"

PartSettingsColors::PartSettingsColors(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partSettingsColors)
{
    ui->setupUi(this);

    m_dark = ThemeManager::instance().dark();
    if (m_dark) {
        ui->btnDark->setChecked(true);
    } else {
        ui->btnLight->setChecked(true);
    }
    updateUIFromGroups();

    connect(ui->btnDark, &QPushButton::clicked, this, [this]() {
        //don't allow unchecking
        if (!ui->btnDark->isChecked()) {
            QSignalBlocker blocker(ui->btnDark);
            ui->btnDark->setChecked(true);
            return;
        }
        updateGroupsFromUI();
        ui->btnLight->setChecked(false);
        m_dark = true;
        updateUIFromGroups();
    });
    connect(ui->btnLight, &QPushButton::clicked, this, [this]() {
        //don't allow unchecking
        if (!ui->btnLight->isChecked()) {
            QSignalBlocker blocker(ui->btnLight);
            ui->btnLight->setChecked(true);
            return;
        }
        updateGroupsFromUI();
        ui->btnDark->setChecked(false);
        m_dark = false;
        updateUIFromGroups();
    });
}

PartSettingsColors::~PartSettingsColors()
{
    delete ui;
}

void PartSettingsColors::setColors(const Groups &groups)
{
    m_groups = groups;
    updateUIFromGroups();
}

PartSettingsColors::Groups PartSettingsColors::colors()
{
    updateGroupsFromUI();

    return m_groups;
}

void PartSettingsColors::updateGroupsFromUI()
{
    Colors& colors = m_dark ? m_groups.dark : m_groups.light;
    colors.toolpathHighlight = ui->clpToolpathHighlightColor->color();
    colors.toolpathZMovement = ui->clpToolpathZMovementColor->color();
    colors.toolpathStart = ui->clpToolpathStartColor->color();
    colors.toolpathEnd = ui->clpToolpathEndColor->color();
    colors.toolpathNormal = ui->clpToolpathNormalColor->color();
    colors.toolpathDrawn = ui->clpToolpathDrawnColor->color();
    colors.toolpathRapidMovement = ui->clpToolpathRapidMovementColor->color();
    colors.visualizerBackground = ui->clpVisualizerBackgroundColor->color();
    colors.visualizerTool = ui->clpVisualizerToolColor->color();
    colors.visualizerCursor = ui->clpVisualizerCursorColor->color();
    colors.visualizerTableGrid = ui->clpVisualizerTableGridColor->color();
}

void PartSettingsColors::updateUIFromGroups()
{
    const Colors& colors = m_dark ? m_groups.dark : m_groups.light;
    ui->clpToolpathHighlightColor->setColor(colors.toolpathHighlight);
    ui->clpToolpathZMovementColor->setColor(colors.toolpathZMovement);
    ui->clpToolpathStartColor->setColor(colors.toolpathStart);
    ui->clpToolpathEndColor->setColor(colors.toolpathEnd);
    ui->clpToolpathNormalColor->setColor(colors.toolpathNormal);
    ui->clpToolpathDrawnColor->setColor(colors.toolpathDrawn);
    ui->clpToolpathRapidMovementColor->setColor(colors.toolpathRapidMovement);
    ui->clpVisualizerBackgroundColor->setColor(colors.visualizerBackground);
    ui->clpVisualizerToolColor->setColor(colors.visualizerTool);
    ui->clpVisualizerCursorColor->setColor(colors.visualizerCursor);
    ui->clpVisualizerTableGridColor->setColor(colors.visualizerTableGrid);
}
