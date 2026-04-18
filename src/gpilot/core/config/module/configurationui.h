// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CONFIGURATIONUI_H
#define CONFIGURATIONUI_H

#include <QObject>
#include <QRect>
#include <QByteArray>
#include <QWidget>
#include "abstractconfigurationmodule.h"

struct ShortcutEntry {
    QString objectName;
    QStringList keySequences; // stored as QKeySequence::toString() strings

    bool operator==(const ShortcutEntry &o) const {
        return objectName == o.objectName && keySequences == o.keySequences;
    }
};

Q_DECLARE_METATYPE(ShortcutEntry)
Q_DECLARE_METATYPE(QList<ShortcutEntry>)

struct DockableState {
    bool visible;
    bool open;
};

class ConfigurationUI : public AbstractConfigurationModule
{
    friend class FrmSettings;

    Q_OBJECT
    // percentage
    Q_PROPERTY(int uiScale MEMBER m_uiScale NOTIFY changed)
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
    Q_PROPERTY(QStringList panelModificationState MEMBER m_panelModificationState NOTIFY changed)
    Q_PROPERTY(QStringList panelDeviceState MEMBER m_panelDeviceState NOTIFY changed)
    Q_PROPERTY(QStringList panelUserState MEMBER m_panelUserState NOTIFY changed)
    Q_PROPERTY(QStringList hiddenPanels MEMBER m_hiddenPanels NOTIFY changed)
    Q_PROPERTY(QStringList collapsedPanels MEMBER m_collapsedPanels NOTIFY changed)
    Q_PROPERTY(QString centralWidget MEMBER m_centralWidget NOTIFY changed)
    Q_PROPERTY(QByteArray mainFormState MEMBER m_mainFormState NOTIFY changed)
    Q_PROPERTY(QByteArray mainFormGeometryData MEMBER m_mainFormGeometryData NOTIFY changed)
    Q_PROPERTY(QByteArray programHeaderState MEMBER m_programHeaderState NOTIFY changed)
    Q_PROPERTY(QList<ShortcutEntry> shortcuts MEMBER m_shortcuts NOTIFY changed)

    public:
        explicit ConfigurationUI(QObject *parent = nullptr);
        QString getSectionName() override { return "baseui.main"; }

        double uiScale() const { return m_uiScale; }
        void setUiScale(int scale) { m_uiScale = scale; emit changed(); }
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
        QStringList panelModificationState() const { return m_panelModificationState; }
        void setPanelModificationState(const QStringList &state) { m_panelModificationState = state; emit changed(); }
        QStringList panelDeviceState() const { return m_panelDeviceState; }
        void setPanelDeviceState(const QStringList &state) { m_panelDeviceState = state; emit changed(); }
        QStringList panelUserState() const { return m_panelUserState; }
        void setPanelUserState(const QStringList &state) { m_panelUserState = state; qDebug() << m_panelUserState; emit changed(); }
        QStringList hiddenPanels() const { return m_hiddenPanels; }
        void setHiddenPanels(const QStringList &panels) { m_hiddenPanels = panels; emit changed(); }
        QStringList collapsedPanels() const { return m_collapsedPanels; }
        void setCollapsedPanels(const QStringList &panels) { m_collapsedPanels = panels; emit changed(); }
        QString centralWidget() const { return m_centralWidget; }
        void setCentralWidget(const QString &widget) { m_centralWidget = widget; emit changed(); }
        QByteArray mainFormState() const { return m_mainFormState; }
        void setMainFormState(const QByteArray &state) { m_mainFormState = state; emit changed(); }
        QByteArray mainFormGeometryData() const { return m_mainFormGeometryData; }
        void setMainFormGeometryData(const QByteArray &data) { m_mainFormGeometryData = data; emit changed(); }
        QByteArray programHeaderState() const { return m_programHeaderState; }
        void setProgramHeaderState(const QByteArray &state) { m_programHeaderState = state; emit changed(); }
        QList<ShortcutEntry> shortcuts() const { return m_shortcuts; }
        void setShortcuts(const QList<ShortcutEntry> &shortcuts) { m_shortcuts = shortcuts; emit changed(); }

        static QList<ShortcutEntry> defaultShortcuts();

    private:
        static const int MAX_RECENT_FILES = 10;
        int m_uiScale;
        QString m_language;
        QStringList m_recentFiles;
        QStringList m_recentHeightmaps;
        QStringList m_panelModificationState;
        QStringList m_panelDeviceState;
        QStringList m_panelUserState;
        QStringList m_hiddenPanels;
        QStringList m_collapsedPanels;
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
        QString m_centralWidget;
        QByteArray m_mainFormState;
        QByteArray m_mainFormGeometryData;
        QByteArray m_programHeaderState;
        QList<ShortcutEntry> m_shortcuts;
};

#endif // CONFIGURATIONUI_H
