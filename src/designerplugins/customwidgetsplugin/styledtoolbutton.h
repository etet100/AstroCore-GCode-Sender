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

class QMenu;

class CUSTOMWIDGETS_DLLSPEC StyledToolButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(QColor backgroundColor READ backColor WRITE setBackColor)
    Q_PROPERTY(QColor foregroundColor READ foreColor WRITE setForeColor)
    Q_PROPERTY(QColor highlightColor READ highlightColor WRITE setHighlightColor)
    Q_PROPERTY(bool invertedDartThemeIconColors MEMBER m_invertedDartThemeIconColors)
    Q_PROPERTY(bool customColors MEMBER m_useCustomColors)
    Q_PROPERTY(int imagePadding MEMBER m_imagePadding)
    Q_PROPERTY(QIcon menuIndicatorIcon READ menuIndicatorIcon WRITE setMenuIndicatorIcon)
    Q_PROPERTY(double indicatorScaleFactor READ indicatorScaleFactor WRITE setIndicatorScaleFactor)
    Q_PROPERTY(int menuIndicatorMargin READ menuIndicatorMargin WRITE setMenuIndicatorMargin)

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

    QIcon menuIndicatorIcon() const;
    void setMenuIndicatorIcon(const QIcon &icon);

    double indicatorScaleFactor() const;
    void setIndicatorScaleFactor(double factor);

    int menuIndicatorMargin() const;
    void setMenuIndicatorMargin(int margin);

    QMenu *buttonMenu() const;
    void setButtonMenu(QMenu *menu);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void hoverChanged(bool hovered);
    void menuRequested();

protected:
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void contextMenuEvent(QContextMenuEvent *e) override;

private:
    Q_DISABLE_COPY(StyledToolButton)

    void paintEvent(QPaintEvent *e) override;
    void paintSimple(QPaintEvent *e);
    void paintMenuIndicator(QPainter &painter);
    void invertIconColors();

    bool hasButtonMenu() const;
    int indicatorSize() const;
    QRect menuIndicatorRect() const;
    bool isMenuIndicatorClick(const QPoint &pos) const;
    void showButtonMenu();

    bool m_hovered = false;
    bool m_useCustomColors = false;
    bool m_invertedDartThemeIconColors = true;
    int m_imagePadding = 0;
    QColor m_backColor;
    QColor m_foreColor;
    QColor m_highlightColor;
    bool m_dark = false;

    QIcon m_menuIndicatorIcon;
    double m_indicatorScaleFactor = 1.0;
    int m_menuIndicatorMargin = 1;
    QMenu *m_menu = nullptr;
    bool m_menuIndicatorPressed = false;
    bool m_menuIndicatorHovered = false;
};

#endif // STYLEDTOOLBUTTON_H
