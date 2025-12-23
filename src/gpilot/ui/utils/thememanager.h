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
    void setFontSize(int);
    // Scale is deduced from the font size. E.g., 7 = 0.9, 8px = 1.0, 9 = 1.1, 10 = 1.2
    // Using font size is deprecated
    float scale();

signals:
    void themeChanged(bool darkMode);
    void fontSizeChanged(int size);

private:
    explicit ThemeManager(QObject *parent = nullptr);
    ~ThemeManager() = default;

    ThemeManager(const ThemeManager&) = delete;
    ThemeManager& operator=(const ThemeManager&) = delete;

    void applyTheme(bool dark);
    void loadStyleSheets(bool dark);

    QApplication *m_app;
    int m_fontSize = -1;
    bool m_darkMode;
};

#endif // THEMEMANAGER_H
