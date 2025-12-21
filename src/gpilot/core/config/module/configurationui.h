// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONUI_H
#define CONFIGURATIONUI_H

#include <QObject>
#include <QRect>
#include <QWidget>
#include "configurationmodule.h"

struct DockableState {
    bool visible;
    bool open;
};

class ConfigurationUI : public ConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    Q_PROPERTY(int fontSize MEMBER m_fontSize NOTIFY changed)
    Q_PROPERTY(QString language MEMBER m_language NOTIFY changed)
    Q_PROPERTY(QStringList recentFiles MEMBER m_recentFiles NOTIFY changed)
    Q_PROPERTY(QStringList recentHeightmaps MEMBER m_recentHeightmaps NOTIFY changed)
    Q_PROPERTY(bool autoScrollGCode MEMBER m_autoScrollGCode NOTIFY changed)
    Q_PROPERTY(QString currentWorkingDirectory MEMBER m_currentWorkingDirectory NOTIFY changed)
    Q_PROPERTY(bool lockWindows MEMBER m_lockWindows NOTIFY changed)
    Q_PROPERTY(bool lockPanels MEMBER m_lockPanels NOTIFY changed)
    Q_PROPERTY(QRect settingsFormGeometry MEMBER m_settingsFormGeometry NOTIFY changed)
    Q_PROPERTY(bool settingsFormMaximized MEMBER m_settingsFormMaximized NOTIFY changed)
    Q_PROPERTY(QRect grblConfigratorFormGeometry MEMBER m_grblConfigratorFormGeometry NOTIFY changed)
    Q_PROPERTY(bool grblConfigratorFormMaximized MEMBER m_grblConfigratorFormMaximized NOTIFY changed)
    Q_PROPERTY(QRect mainFormGeometry MEMBER m_mainFormGeometry NOTIFY changed)
    Q_PROPERTY(bool mainFormMaximized MEMBER m_mainFormMaximized NOTIFY changed)
    Q_PROPERTY(QList<int> settingsFormSlicerSizes MEMBER m_settingsFormSlicerSizes NOTIFY changed)
    Q_PROPERTY(bool darkMode MEMBER m_darkMode NOTIFY changed)

    public:
        explicit ConfigurationUI(QObject *parent);
        QString getSectionName() override { return "baseui.main"; }

        int fontSize() const { return m_fontSize; }
        QString language() const { return m_language; }
        QStringList recentFiles() const { return m_recentFiles; }
        QStringList recentHeightmaps() const { return m_recentHeightmaps; }
        bool autoScrollGCode() const { return m_autoScrollGCode; }
        void setAutoScrollGCode(bool autoScrollGCode) { m_autoScrollGCode = autoScrollGCode; emit changed(); }
        QString currentWorkingDirectory() const { return m_currentWorkingDirectory; }
        void currentWorkingDirectory(const QString &directory) { m_currentWorkingDirectory = directory; emit changed(); }
        void addRecentFile(const QString &fileName);
        void addRecentHeightmap(const QString &fileName);
        bool hasAnyRecentFiles() const;
        bool hasAnyRecentHeightmaps() const;
        void clearRecentFiles();
        void clearRecentHeightmaps();
        bool lockWindows() const { return m_lockWindows; }
        void setLockWindows(bool lockWindows) { m_lockWindows = lockWindows; emit changed(); }
        bool lockPanels() const { return m_lockPanels; }
        void setLockPanels(bool lockPanels) { m_lockPanels = lockPanels; emit changed(); }
        QRect settingsFormGeometry() const { return m_settingsFormGeometry; }
        void setSettingsFormGeometry(const QWidget *widget);
        bool settingsFormMaximized() const { return m_settingsFormMaximized; }
        QRect grblConfigratorFormGeometry() const { return m_grblConfigratorFormGeometry; }
        void setGrblConfigratorFormGeometry(const QWidget *widget);
        bool grblConfigratorFormMaximized() const { return m_grblConfigratorFormMaximized; }
        QRect mainFormGeometry() const { return m_mainFormGeometry; }
        void setMainFormGeometry(const QWidget *widget);
        bool mainFormMaximized() const { return m_mainFormMaximized; }
        QList<int> settingsFormSlicerSizes() const { return m_settingsFormSlicerSizes; }
        void setSettingsFormSlicerSizes(QList<int> sizes) { m_settingsFormSlicerSizes = sizes; emit changed(); }
        bool darkTheme() const { return m_darkMode; }
        void setDarkMode(bool darkMode) { m_darkMode = darkMode; emit changed(); }

    private:
        static const int MAX_RECENT_FILES = 10;
        int m_fontSize;
        QString m_language;
        QStringList m_recentFiles;
        QStringList m_recentHeightmaps;
        bool m_autoScrollGCode;
        QString m_currentWorkingDirectory;
        bool m_lockWindows;
        bool m_lockPanels;
        QRect m_mainFormGeometry;
        bool m_mainFormMaximized;
        QRect m_settingsFormGeometry;
        bool m_settingsFormMaximized;
        QRect m_grblConfigratorFormGeometry;
        bool m_grblConfigratorFormMaximized;
        QList<int> m_settingsFormSlicerSizes;
        bool m_darkMode;
};

#endif // CONFIGURATIONUI_H
