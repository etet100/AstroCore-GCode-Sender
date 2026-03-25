// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "styledtoolbutton.h"
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QStyleHints>

StyledToolButton::StyledToolButton(QWidget *parent) : QToolButton(parent)
{
    m_backColor = palette().color(QPalette::Button);
    m_foreColor = palette().color(QPalette::ButtonText);
    m_highlightColor = palette().color(QPalette::Highlight);
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
    QToolButton::leaveEvent(e);
    emit hoverChanged(false);
}

void StyledToolButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

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

void StyledToolButton::invertIconColors()
{
    QIcon icon = this->icon();
    QImage img = icon.pixmap(icon.actualSize(iconSize()), QIcon::Normal).toImage();

    setIcon(QIcon(QPixmap::fromImage(img)));
}
