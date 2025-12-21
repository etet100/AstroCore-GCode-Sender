// This file is a part of "G-Pilot" application.
// Copyright 2024 BTS

#include "thememanager.h"
#include "phantomstyle/src/phantom/phantomstyle.h"
#include <QFile>
#include <QStyleHints>
#include <QDebug>
#include <QRegularExpression>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_app(nullptr)
    , m_darkMode(false)
{
}

ThemeManager& ThemeManager::instance()
{
    static ThemeManager instance;

    return instance;
}

void ThemeManager::initialize(QApplication *app, bool darkMode)
{
    m_app = app;
    m_darkMode = darkMode;
    applyTheme(darkMode);
}

void ThemeManager::setDarkMode(bool dark)
{
    if (m_darkMode == dark) {
        return;
    }

    m_darkMode = dark;
    applyTheme(dark);

    emit themeChanged(dark);
}

void ThemeManager::setFontSize(int size)
{
    m_app->setStyleSheet(QString(m_app->styleSheet()).replace(
        QRegularExpression("/\\* mainfontsize \\*/ font-size:[^;^\\}]+"),
        QString("/* mainfontsize */ font-size: %1pt").arg(size))
    );

    emit fontSizeChanged(size);
}

void ThemeManager::applyTheme(bool dark)
{
    if (!m_app) {
        qWarning() << "[ThemeManager] QApplication not set";
        return;
    }

    // Set color scheme
    m_app->styleHints()->setColorScheme(dark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);

    // Set style
    m_app->setStyle(new PhantomStyle());

    // Set palette
    QPalette palette;
    palette.setColor(QPalette::Highlight, QColor(204, 204, 254));
    palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
    m_app->setPalette(palette);

    // Load stylesheets
    loadStyleSheets(dark);

    // Update app property for legacy code
    m_app->setProperty("dark", dark);
}

void ThemeManager::loadStyleSheets(bool dark)
{
    if (!m_app) {
        qWarning() << "[ThemeManager] QApplication not set";
        return;
    }

    QFile stylesheetFile(":/stylesheets/main.qss");
    if (!stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[ThemeManager] Cannot open main stylesheet file";
        return;
    }

    QString stylesheet = stylesheetFile.readAll();
    stylesheetFile.close();

    if (dark) {
        stylesheetFile.setFileName(":/stylesheets/dark.qss");
    } else {
        stylesheetFile.setFileName(":/stylesheets/light.qss");
    }

    if (!stylesheetFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[ThemeManager] Cannot open theme stylesheet file";
        return;
    }

    stylesheet += "\n\n" + stylesheetFile.readAll();
    stylesheetFile.close();

    m_app->setStyleSheet(stylesheet);
}
