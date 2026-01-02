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

    void initialize(QApplication *app, bool dark);
    void setDark(bool dark);
    bool dark() const { return m_dark; }
    void setFontSize(int, bool force = false);
    // Scale is deduced from the font size. E.g., 7 = 0.9, 8px = 1.0, 9 = 1.1, 10 = 1.2
    // Using font size is deprecated
    float scale();
    void processQssTemplate(QWidget *widget);

signals:
    void themeChanged(bool dark);
    void scaleChanged(float scale);
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
    bool m_dark;
};

#endif // THEMEMANAGER_H
