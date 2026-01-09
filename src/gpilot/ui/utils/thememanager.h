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
    void setScale(int, bool force = false);
    void increaseScale();
    void decreaseScale();
    int scale();
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
    int m_scale = -1;
    bool m_dark;

    int scaleToFontSize(int scale);
};

#endif // THEMEMANAGER_H
