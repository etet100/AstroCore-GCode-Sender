// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "styledtoolbutton.h"
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QPainterPath>
#include <QStyleHints>
#include <QStyleOptionToolButton>
#include <QStylePainter>
#include <QResizeEvent>

StyledToolButton::StyledToolButton(QWidget *parent) : QToolButton(parent)
{
    m_backColor = palette().color(QPalette::Button);
    m_foreColor = palette().color(QPalette::ButtonText);
    m_highlightColor = palette().color(QPalette::Highlight);
    setMouseTracking(true);
}

bool StyledToolButton::isHover()
{
    return m_hovered;
}

void StyledToolButton::enterEvent(QEnterEvent *e)
{
    m_hovered = true;
    QToolButton::enterEvent(e);
    emit hoverChanged(true);
}

void StyledToolButton::leaveEvent(QEvent *e)
{
    m_hovered = false;
    if (m_menuIndicatorHovered) {
        m_menuIndicatorHovered = false;
        unsetCursor();
    }
    QToolButton::leaveEvent(e);
    emit hoverChanged(false);
}

void StyledToolButton::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && isMenuIndicatorClick(e->position().toPoint())) {
        m_menuIndicatorPressed = true;
        e->accept();

        return;
    }

    m_menuIndicatorPressed = false;
    QToolButton::mousePressEvent(e);
}

void StyledToolButton::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && m_menuIndicatorPressed) {
        m_menuIndicatorPressed = false;
        if (isMenuIndicatorClick(e->position().toPoint())) {
            showButtonMenu();
        }
        e->accept();

        return;
    }

    m_menuIndicatorPressed = false;
    QToolButton::mouseReleaseEvent(e);
}

void StyledToolButton::mouseMoveEvent(QMouseEvent *e)
{
    bool hovered = isMenuIndicatorClick(e->position().toPoint());
    if (hovered != m_menuIndicatorHovered) {
        m_menuIndicatorHovered = hovered;
        setCursor(hovered ? Qt::ArrowCursor : Qt::PointingHandCursor);
        update();
    }

    QToolButton::mouseMoveEvent(e);
}

QSize StyledToolButton::sizeHint() const
{
    QSize base = QToolButton::sizeHint();
    if (hasButtonMenu()) {
        base.rwidth() += indicatorSize() * 0.6;
    }

    return base;
}

QSize StyledToolButton::minimumSizeHint() const
{
    QSize base = QToolButton::minimumSizeHint();
    if (hasButtonMenu()) {
        base.rwidth() += indicatorSize() * 0.6;
    }

    return base;
}

void StyledToolButton::paintSimple(QPaintEvent *)
{
    if (!icon().isNull()) {
        const bool isDark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
        if (m_dark != isDark) {
            m_dark = isDark;
            // QIcon ico = icon();
            // QSize sz = iconSize().shrunkBy(QMargins(m_imagePadding, m_imagePadding, m_imagePadding, m_imagePadding));
            // QImage img = ico.pixmap(ico.actualSize(sz), QIcon::Normal).toImage();
            // img.invertPixels();
            // setIcon(QIcon(QPixmap::fromImage(img)));
            invertIconColors();
        }
    }

    QStylePainter painter(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    // Suppress Qt's built-in menu indicator — we draw our own
    opt.features &= ~QStyleOptionToolButton::HasMenu;

    painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

    QStyleOptionToolButton labelOpt = opt;
    if (hasButtonMenu()) {
        labelOpt.rect.adjust(0, 0, -indicatorSize() * 0.6, 0);
    }
    painter.drawControl(QStyle::CE_ToolButtonLabel, labelOpt);

    if (!hasButtonMenu()) {
        return;
    }

    paintMenuIndicator(painter);
}

void StyledToolButton::paintMenuIndicator(QPainter &painter)
{
    painter.setRenderHint(QPainter::Antialiasing);

    QRect areaRect = menuIndicatorRect();
    qreal radius = areaRect.height() * 0.2;

    QColor bgColor = palette().color(QPalette::ButtonText);
    if (m_menuIndicatorHovered) {
        bgColor.setAlphaF(0.18);
    } else {
        bgColor.setAlphaF(0.10);
    }

    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(areaRect), radius, radius);
    painter.fillPath(bgPath, bgColor);

    QRect iconRect = areaRect.adjusted(2, 2, -2, -2);

    if (!m_menuIndicatorIcon.isNull()) {
        QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        m_menuIndicatorIcon.paint(&painter, iconRect, Qt::AlignCenter, mode);
    } else {
        QColor arrowColor = palette().color(
            isEnabled() ? QPalette::Active : QPalette::Disabled,
            QPalette::ButtonText
        );
        arrowColor.setAlphaF(m_menuIndicatorHovered ? 0.9 : 0.6);

        painter.setPen(Qt::NoPen);
        painter.setBrush(arrowColor);

        QPointF center = QRectF(iconRect).center();
        qreal halfW = iconRect.width() * 0.4;
        qreal halfH = iconRect.height() * 0.35;

        QPolygonF triangle;
        triangle << QPointF(center.x() - halfW, center.y() - halfH * 0.5)
                 << QPointF(center.x() + halfW, center.y() - halfH * 0.5)
                 << QPointF(center.x(), center.y() + halfH * 0.8);

        painter.drawPolygon(triangle);
    }
}

void StyledToolButton::paintEvent(QPaintEvent *e)
{
    // Simple version
    paintSimple(e);
    return;

    const qreal radius = 4.0;
    const bool isDark = QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark;
    const bool active = isEnabled();
    const bool down = isDown() || isChecked();
    const QPalette::ColorGroup cg = active ? QPalette::Active : QPalette::Disabled;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Border: matches PhantomStyle's S_window_outline (Window darkened ~10% lightness)
    auto windowOutline = [&]() {
        QColor c = palette().color(cg, QPalette::Window);
        float h, s, l, a;
        c.getHslF(&h, &s, &l, &a);
        l = qPow(qBound(0.0, qPow(l, 1.0 / 3.0) - 0.08, 1.0), 3.0);
        return QColor::fromHslF(h, s, l, a);
    };

    // Fill
    QColor fill;
    if (m_useCustomColors) {
        fill = m_backColor;
    } else {
        fill = palette().color(cg, QPalette::Button);
        if (active) {
            if (down) {
                fill = isDark ? fill.lighter(115) : fill.darker(110);
            } else if (m_hovered) {
                fill = isDark ? fill.lighter(108) : fill.darker(105);
            }
        }
    }

    const QRectF r = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setBrush(fill);
    painter.setPen(QPen(windowOutline(), 1.0));
    painter.drawRoundedRect(r, radius, radius);

    // Shift content 2px down when pressed, like standard buttons
    QRect innerRect = rect().adjusted(1, 1, -1, -1);
    if (down && active) {
        innerRect.translate(0, 2);
    }

    if (!icon().isNull()) {
        QIcon ico = icon();
        QSize sz = iconSize().shrunkBy(QMargins(m_imagePadding, m_imagePadding, m_imagePadding, m_imagePadding));
        QImage img = ico.pixmap(ico.actualSize(sz), QIcon::Normal).toImage();
        if (m_invertedDartThemeIconColors && isDark) {
            img.invertPixels();
        }
        if (!active) {
            painter.setOpacity(0.35);
        }
        painter.drawImage(
            QRect(innerRect.x() + (innerRect.width()  - sz.width())  / 2,
                  innerRect.y() + (innerRect.height() - sz.height()) / 2,
                  sz.width(), sz.height()),
            img
        );
        if (!active) {
            painter.setOpacity(1.0);
        }
    } else {
        QColor textColor = m_useCustomColors ? m_foreColor : palette().color(cg, QPalette::ButtonText);
        painter.setPen(textColor);
        painter.drawText(innerRect, Qt::AlignCenter, text());
    }
}

QColor StyledToolButton::highlightColor() const
{
    return m_highlightColor;
}

void StyledToolButton::setHighlightColor(const QColor &highlightColor)
{
    m_highlightColor = highlightColor;
}

bool StyledToolButton::useCustomColors() const
{
    return m_useCustomColors;
}

void StyledToolButton::setUseCustomColors(bool use)
{
    m_useCustomColors = use;
}

QColor StyledToolButton::foreColor() const
{
    return m_foreColor;
}

void StyledToolButton::setForeColor(const QColor &foreColor)
{
    m_foreColor = foreColor;
}

QColor StyledToolButton::backColor() const
{
    return m_backColor;
}

void StyledToolButton::setBackColor(const QColor &backColor)
{
    m_backColor = backColor;
}

QIcon StyledToolButton::menuIndicatorIcon() const
{
    return m_menuIndicatorIcon;
}

void StyledToolButton::setMenuIndicatorIcon(const QIcon &icon)
{
    m_menuIndicatorIcon = icon;
    update();
}

double StyledToolButton::indicatorScaleFactor() const
{
    return m_indicatorScaleFactor;
}

void StyledToolButton::setIndicatorScaleFactor(double factor)
{
    m_indicatorScaleFactor = factor;
    update();
}

int StyledToolButton::menuIndicatorMargin() const
{
    return m_menuIndicatorMargin;
}

void StyledToolButton::setMenuIndicatorMargin(int margin)
{
    m_menuIndicatorMargin = margin;
    update();
}

bool StyledToolButton::invertedDartThemeIconColors() const
{
    return m_invertedDartThemeIconColors;
}

void StyledToolButton::setInvertedDartThemeIconColors(bool inverted)
{
    m_invertedDartThemeIconColors = inverted;
    update();
}

QMenu *StyledToolButton::buttonMenu() const
{
    return m_menu;
}

void StyledToolButton::setButtonMenu(QMenu *menu)
{
    m_menu = menu;
}

void StyledToolButton::invertIconColors()
{
    QIcon icon = this->icon();
    QImage img = icon.pixmap(icon.actualSize(iconSize()), QIcon::Normal).toImage();
    img.invertPixels();

    setIcon(QIcon(QPixmap::fromImage(img)));
}

int StyledToolButton::indicatorSize() const
{
    return qRound(fontMetrics().height() * 0.85 * m_indicatorScaleFactor);
}

QRect StyledToolButton::menuIndicatorRect() const
{
    int size = indicatorSize();

    return QRect(
        width() - size - m_menuIndicatorMargin,
        height() - size - m_menuIndicatorMargin,
        size,
        size
    );
}

bool StyledToolButton::isMenuIndicatorClick(const QPoint &pos) const
{
    return hasButtonMenu() && menuIndicatorRect().contains(pos);
}

void StyledToolButton::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menuToShow = m_menu ? m_menu : QToolButton::menu();
    if (menuToShow || receivers(SIGNAL(menuRequested())) > 0) {
        e->accept();
        showButtonMenu();
    } else {
        // Handles Qt::CustomContextMenu — Qt emits customContextMenuRequested
        QToolButton::contextMenuEvent(e);
    }
}

bool StyledToolButton::hasButtonMenu() const
{
    return m_menu != nullptr
        || QToolButton::menu() != nullptr
        || receivers(SIGNAL(menuRequested())) > 0
        || contextMenuPolicy() == Qt::CustomContextMenu;
}

void StyledToolButton::showButtonMenu()
{
    emit menuRequested();

    QMenu *menuToShow = m_menu ? m_menu : QToolButton::menu();
    if (menuToShow) {
        QPoint pos = mapToGlobal(QPoint(width() - menuToShow->sizeHint().width(), height()));
        menuToShow->popup(pos);
    } else if (contextMenuPolicy() == Qt::CustomContextMenu) {
        emit customContextMenuRequested(QPoint(width(), height()));
    }
}
