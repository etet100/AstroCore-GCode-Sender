// This file is a part of "G-Pilot" application.
// Copyright 2024 BTS

#include "thememanager.h"
#include "phantomstyle/src/phantom/phantomstyle.h"
#include <QFile>
#include <QStyleHints>
#include <QDebug>
#include <QWidget>
#include <QRegularExpression>

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_app(nullptr)
    , m_dark(false)
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
    m_dark = darkMode;
    applyTheme(darkMode);
}

void ThemeManager::setDark(bool dark)
{
    if (m_dark == dark) {
        return;
    }

    m_dark = dark;
    applyTheme(dark);
    // Re-apply font size after stylesheet change
    if (m_fontSize > 0) {
        setFontSize(m_fontSize, true);
    }

    emit themeChanged(dark);
}

void ThemeManager::setFontSize(int size, bool force)
{
    if (!force && m_fontSize == size) {
        return;
    }

    m_app->setStyleSheet(QString(m_app->styleSheet()).replace(
        QRegularExpression("/\\* mainfontsize \\*/ font-size:[^;^\\}]+"),
        QString("/* mainfontsize */ font-size: %1pt").arg(size))
    );

    // Do not emit signal if size did not change, even if forced
    if (m_fontSize == size) {
        return;
    }

    m_fontSize = size;
    emit fontSizeChanged(size);
    emit scaleChanged(scale());
}

float ThemeManager::scale()
{
    // Mapping font size to scale factor is something to tune later
    switch (m_fontSize) {
        case 7: return 0.9f;
        case 9: return 1.1f;
        case 10: return 1.2f;
        case 11: return 1.3f;
        case 12: return 1.4f;
        case 8:
        default:
            return 1.0f;
    }
}

void ThemeManager::processQssTemplate(QWidget *widget)
{
    QVariant qssProp = widget->property("qss");
    QString qss;
    if (!qssProp.isValid()) {
        qss = widget->styleSheet();
        widget->setProperty("qss", qss);
    } else {
        qss = qssProp.toString();
    }

    // Extremely simple template processing, have to be extended!!
    qss = qss.replace("{font-size}", QString::number(m_fontSize))
        .replace("{0.5*font-size}", QString::number(0.5 * m_fontSize))
        .replace("{1.5*font-size}", QString::number(1.5 * m_fontSize))
        .replace("{2*font-size}", QString::number(2 * m_fontSize))
        .replace("{2.5*font-size}", QString::number(2.5 * m_fontSize))
        .replace("{scale}", QString::number(scale()))
        .replace("{scale/2}", QString::number(scale() / 2.0))
        .replace("{1.5*scale}", QString::number(scale() * 1.0))
        .replace("{2*scale}", QString::number(scale() * 2.0))
        .replace("{2.5*scale}", QString::number(scale() * 2.5));

    widget->setStyleSheet(qss);
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

#ifdef UNIX
    // Any better way to do this cross-platform?
    QPalette pall = m_app->palette();
    if (dark) {
        pall.setColor(QPalette::Window, QColor(30,30,30));
        pall.setColor(QPalette::WindowText, Qt::white);
        pall.setColor(QPalette::Base, QColor(20,20,20));
        pall.setColor(QPalette::Text, Qt::white);
        pall.setColor(QPalette::Button, QColor(45,45,45));
        pall.setColor(QPalette::ButtonText, Qt::white);
    } else {
        pall.setColor(QPalette::Window, QColor(240,240,240));
        pall.setColor(QPalette::WindowText, Qt::black);
        pall.setColor(QPalette::Base, Qt::white);
        pall.setColor(QPalette::Text, Qt::black);
        pall.setColor(QPalette::Button, QColor(225,225,225));
        pall.setColor(QPalette::ButtonText, Qt::black);
    }
    m_app->setPalette(pall);
#endif

    // Load stylesheets
    loadStyleSheets(dark);

    // Update app property for legacy code
    m_app->setProperty("dark", dark ? "true" : "false");
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
