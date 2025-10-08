// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "configurationui.h"

const QMap<QString,QVariant> DEFAULTS = {
    {"fontSize", 8},
    {"language", "en"},
    {"recentFiles", QStringList{}},
    {"recentHeightmaps", QStringList{"map1.txt", "map2.txt", "map3.txt"}},
    {"autoScrollGCode", true},
    {"lastFileOpenDir", ""},
    {"lockWindows", false},
    {"lockPanels", false},
    // -1 is a special value to indicate that the window should be centered
    // -1 is a special value to indicate that the window should be maximized
    {"mainFormGeometry", QVariantMap({{"x", 0}, {"y", 0}, {"width", -1}, {"height", -1}})},
    {"mainFormMaximized", true},
    {"settingsFormGeometry", QVariantMap({{"x", -1}, {"y", -1}, {"width", 800}, {"height", 600}})},
    {"settingsFormMaximized", false},
    {"settingsFormSlicerSizes", QVariantList{20, 200}},
    {"grblConfigratorFormGeometry", QVariantMap({{"x", -1}, {"y", -1}, {"width", 800}, {"height", 600}})},
    {"grblConfigratorFormMaximized", false},
    {"spindleSpeedRange", QVariantMap({{"min", 0}, {"max", 100}})},
    {"darkTheme", false}
};

ConfigurationUI::ConfigurationUI(QObject *parent) : ConfigurationModule(parent, DEFAULTS)
{
}

void ConfigurationUI::addRecentFile(const QString &fileName)
{
    m_recentFiles.removeAll(fileName);
    m_recentFiles.append(fileName);
    m_recentFiles = m_recentFiles.mid(qMax(0, m_recentFiles.size() - MAX_RECENT_FILES));
}

void ConfigurationUI::addRecentHeightmap(const QString &fileName)
{
    m_recentHeightmaps.removeAll(fileName);
    m_recentHeightmaps.append(fileName);
    m_recentHeightmaps = m_recentHeightmaps.mid(qMax(0, m_recentHeightmaps.size() - MAX_RECENT_FILES));
}

bool ConfigurationUI::hasAnyRecentFiles() const
{
    return !m_recentFiles.empty();
}

bool ConfigurationUI::hasAnyRecentHeightmaps() const
{
    return !m_recentHeightmaps.empty();
}

void ConfigurationUI::clearRecentFiles()
{
    m_recentFiles.clear();
}

void ConfigurationUI::clearRecentHeightmaps()
{
    m_recentHeightmaps.clear();
}

void ConfigurationUI::setSettingsFormGeometry(const QWidget *widget)
{
    m_settingsFormGeometry = widget->frameGeometry();
    m_settingsFormMaximized = widget->isMaximized();

    emit changed();
}

void ConfigurationUI::setGrblConfigratorFormGeometry(const QWidget *widget)
{
    m_grblConfigratorFormGeometry = widget->frameGeometry();
    m_grblConfigratorFormMaximized = widget->isMaximized();

    emit changed();
}

void ConfigurationUI::setMainFormGeometry(const QWidget *widget)
{
    m_mainFormGeometry = widget->frameGeometry();
    m_mainFormMaximized = widget->isMaximized();

    emit changed();
}
