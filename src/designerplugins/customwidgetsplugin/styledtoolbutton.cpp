// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "styledtoolbutton.h"
#include <QDebug>
#include <QEvent>
#include <QGuiApplication>
#include <QStyleHints>

StyledToolButton::StyledToolButton(QWidget *parent) : QToolButton(parent)
{
    if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        invertIconColors();
        m_backColor = palette().color(QPalette::Button).darker(120);
    } else {
        m_backColor = palette().color(QPalette::Button);
    }
    m_foreColor = palette().color(QPalette::ButtonText);
    m_highlightColor = QColor(127, 211, 255).darker(120);
}

bool StyledToolButton::isHover()
{
    return m_hovered;
}

void StyledToolButton::enterEvent(QEnterEvent *e)
{
    m_hovered = true;

    QToolButton::enterEvent(e);
}

void StyledToolButton::leaveEvent(QEvent *e)
{
    m_hovered = false;

    QToolButton::leaveEvent(e);
}

void StyledToolButton::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    const int borderWidth = 4;
    const int borderRadius = 5;

    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    // Highlight
    QPen highlightPen;

    QColor highlightColor;
    if (m_useCustomColors) {
        highlightColor = m_highlightColor;
    } else {
        QColor baseColor = palette().color(QPalette::Button);
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            highlightColor = baseColor.lighter(150);
        } else {
            highlightColor = baseColor.darker(150);
        }
    }

    if (this->isHover() && !this->isDown() && !this->isChecked()) {
        highlightPen.setColor(highlightColor.lighter(110));
    } else {
        highlightPen.setColor(highlightColor);
    }

    // Internal - secondary border
    // painter.setPen(highlightPen);
    // painter.drawRoundedRect(1, 1, this->width() - 2, this->height() - 2, borderRadius - 1, borderRadius - 1);

    // Border
    QPen pen(this->isEnabled() ? palette().color(QPalette::Shadow) : palette().color(QPalette::Mid));

    if ((this->isDown() || this->isChecked()) && this->isEnabled()) pen.setColor(palette().color(QPalette::Dark));

    pen.setWidth(2);
    pen.setCapStyle(Qt::SquareCap);
    painter.setPen(pen);

    painter.drawLine(borderRadius, 0, width() - borderRadius, 0);
    painter.drawLine(borderRadius, height(), width() - borderRadius, height());
    painter.drawLine(0, borderRadius, 0, height() - borderRadius);
    painter.drawLine(width(), borderRadius, width(), height() - borderRadius);

    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawArc(0, 0, borderRadius * 2, borderRadius * 2, 90 * 16, 90 * 16);
    painter.drawArc(width() - borderRadius * 2, 0, borderRadius * 2, borderRadius * 2, 0 * 16, 90 * 16);
    painter.drawArc(0, height() - borderRadius * 2, borderRadius * 2, borderRadius * 2, 180 * 16, 90 * 16);
    painter.drawArc(width() - borderRadius * 2, height() - borderRadius * 2, borderRadius * 2, borderRadius * 2, 270 * 16, 90 * 16);

    // Background border
    QColor backColor;
    if (m_useCustomColors) {
        backColor = m_backColor;
    } else {
        backColor = palette().color(QPalette::Button);
        if (QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            backColor = backColor.darker(120);
        }
    }
    QLinearGradient backGradient(width() / 2, height() / 2, width() / 2, height());
    backGradient.setColorAt(0, this->isEnabled() ? backColor : palette().color(QPalette::Button));
    backGradient.setColorAt(1, this->isEnabled() ? backColor.darker(130) : palette().color(QPalette::Button).darker(130));
    QBrush backBrush(backGradient);
    painter.setBrush(backBrush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(borderWidth - 1, borderWidth - 1, width() - borderWidth * 2 + 2, height() - borderWidth * 2 + 2, 2, 2);

    // Background
    painter.setBrush(this->isEnabled() ? backColor : palette().color(QPalette::Button));
    painter.setPen(Qt::NoPen);
    painter.drawRect(borderWidth, borderWidth, width() - borderWidth * 2, height() - borderWidth * 2);

    // Icon/text rect
    QRect innerRect(borderWidth, borderWidth, width() - borderWidth * 2, height() - borderWidth * 2);
    if (this->isDown() || this->isChecked()) {
        innerRect.setLeft(innerRect.left() + 2);
        innerRect.setTop(innerRect.top() + 2);
    }

    // Icon
    if (!this->icon().isNull()) {
        QIcon icon = this->icon();
        QSize iconSize = this->iconSize();
        QImage img = icon.pixmap(icon.actualSize(iconSize), this->isEnabled() ? QIcon::Normal : QIcon::Disabled).toImage();
        if (m_invertedDartThemeIconColors && QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            img.invertPixels();
        }

        painter.drawImage(
            QRect(
                innerRect.x() + (innerRect.width() - iconSize.width()) / 2,
                innerRect.y() + (innerRect.height() - iconSize.height()) / 2,
                iconSize.width(),
                iconSize.height()
            ),
            img
        );
    } else {
        // Text
        QColor textColor = m_useCustomColors ? m_foreColor : palette().color(QPalette::ButtonText);
        painter.setPen(this->isEnabled() ? textColor : palette().color(QPalette::ButtonText));
        painter.drawText(innerRect, Qt::AlignCenter, this->text());
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
