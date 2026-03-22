// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSCOLORS_H
#define PARTSETTINGSCOLORS_H

#include <QWidget>

namespace Ui {
class partSettingsColors;
}

class PartSettingsColors : public QWidget
{
        Q_OBJECT

    public:
        struct Colors
        {
                QColor toolpathHighlight;
                QColor toolpathZMovement;
                QColor toolpathStart;
                QColor toolpathEnd;
                QColor toolpathNormal;
                QColor toolpathDrawn;
                QColor toolpathRapidMovement;
                QColor visualizerBackground;
                QColor visualizerTool;
                QColor visualizerCursor;
                QColor visualizerTableGrid;
        };

        struct Groups
        {
                Colors light;
                Colors dark;
        };

        explicit PartSettingsColors(QWidget *parent = nullptr);
        ~PartSettingsColors();
        void setColors(const Groups& groups);
        Groups colors();

    private:
        Ui::partSettingsColors *ui;
        Groups m_groups;
        bool m_dark;
        void updateGroupsFromUI();
        void updateUIFromGroups();
};

#endif // PARTSETTINGSCOLORS_H

