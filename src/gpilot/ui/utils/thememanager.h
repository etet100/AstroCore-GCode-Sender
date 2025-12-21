// This file is a part of "G-Pilot" application.
// Copyright 2024 BTS

#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QPalette>

class ThemeManager : public QObject
{
    Q_OBJECT

public:
    static ThemeManager& instance();

    void initialize(QApplication *app, bool darkMode);
    void setDarkMode(bool dark);
    bool isDarkMode() const { return m_darkMode; }

signals:
    void themeChanged(bool darkMode);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void applyTheme(bool dark);
    void loadStyleSheets(bool dark);

    QApplication *m_app;
    bool m_darkMode;
};

#endif // THEMEMANAGER_H
