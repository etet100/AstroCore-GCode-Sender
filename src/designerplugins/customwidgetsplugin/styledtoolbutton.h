// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef STYLEDTOOLBUTTON_H
#define STYLEDTOOLBUTTON_H

#include <QWidget>
#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QToolButton>
#include "customwidgetsshared.h"

class CUSTOMWIDGETS_DLLSPEC StyledToolButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backColor WRITE setBackColor)
    Q_PROPERTY(QColor foregroundColor READ foreColor WRITE setForeColor)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor)
    Q_PROPERTY(bool invertedDartThemeIconColors MEMBER m_invertedDartThemeIconColors)
    Q_PROPERTY(bool customColors MEMBER m_useCustomColors)
    Q_PROPERTY(int imagePadding MEMBER m_imagePadding)

public:
    explicit StyledToolButton(QWidget *parent = 0);

    bool isHover();

    QColor backColor() const;
    void setBackColor(const QColor &backColor);

    QColor foreColor() const;
    void setForeColor(const QColor &foreColor);

    QColor highlightColor() const;
    void setHighlightColor(const QColor &highlightColor);

    bool useCustomColors() const;
    void setUseCustomColors(bool use);

signals:
    void hoverChanged(bool hovered);

protected:
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    Q_DISABLE_COPY(StyledToolButton)

    void paintEvent(QPaintEvent *e) override;

    bool m_hovered = false;
    bool m_useCustomColors = false;
    bool m_invertedDartThemeIconColors = true;
    int m_imagePadding = 0;
    QColor m_backColor;
    QColor m_foreColor;
    QColor m_highlightColor;

    void invertIconColors();
};

#endif // STYLEDTOOLBUTTON_H
