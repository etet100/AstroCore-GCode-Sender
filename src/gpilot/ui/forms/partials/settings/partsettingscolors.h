// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef PARTSETTINGSCOLORS_H
#define PARTSETTINGSCOLORS_H

#include <QWidget>

namespace Ui {
class partSettingsColors;
}

class partSettingsColors : public QWidget
{
        Q_OBJECT

    public:
        explicit partSettingsColors(QWidget *parent = nullptr);
        ~partSettingsColors();
        void setToolpathHighlightColor(const QColor &color);
        void setToolpathZMovementColor(const QColor &color);
        void setToolpathStartColor(const QColor &color);
        void setToolpathEndColor(const QColor &color);
        void setToolpathNormalColor(const QColor &color);
        void setToolpathDrawnColor(const QColor &color);
        void setToolpathRapidMovementColor(const QColor &color);
        void setVisualizerBackgroundColor(const QColor &color);
        void setVisualizerTextColor(const QColor &color);
        void setVisualizerToolColor(const QColor &color);
        void setVisualizerTableGridColor(const QColor &color);
        QColor toolpathHighlightColor() const;
        QColor toolpathZMovementColor() const;
        QColor toolpathStartColor() const;
        QColor toolpathEndColor() const;
        QColor toolpathNormalColor() const;
        QColor toolpathDrawnColor() const;
        QColor toolpathRapidMovementColor() const;
        QColor visualizerBackgroundColor() const;
        QColor visualizerTextColor() const;
        QColor visualizerToolColor() const;
        QColor visualizerTableGridColor() const;

    private:
        Ui::partSettingsColors *ui;
};

#endif // PARTSETTINGSCOLORS_H

