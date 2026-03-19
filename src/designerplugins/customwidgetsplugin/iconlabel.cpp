#include "iconlabel.h"

#include <QEvent>
#include <QFontMetrics>
#include <QGuiApplication>
#include <QPainter>
#include <QStyleHints>

IconLabel::IconLabel(QWidget *parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QString IconLabel::text() const
{
    return m_text;
}

void IconLabel::setText(const QString &text)
{
    m_text = text;
    updateGeometry();
    update();
}

QIcon IconLabel::icon() const
{
    return m_icon;
}

void IconLabel::setIcon(const QIcon &icon)
{
    m_icon = icon;
    updateGeometry();
    update();
}

QSize IconLabel::iconSize() const
{
    return m_iconSize;
}

void IconLabel::setIconSize(const QSize &size)
{
    m_iconSize = size;
    updateGeometry();
    update();
}

int IconLabel::spacing() const
{
    return m_spacing;
}

void IconLabel::setSpacing(int spacing)
{
    m_spacing = spacing;
    updateGeometry();
    update();
}

bool IconLabel::invertIconColors() const
{
    return m_invertIconColors;
}

void IconLabel::setInvertIconColors(bool invert)
{
    m_invertIconColors = invert;
    update();
}

QSize IconLabel::sizeHint() const
{
    QFontMetrics fm(font());
    int iconW = m_icon.isNull() ? 0 : m_iconSize.width();
    int textW = m_text.isEmpty() ? 0 : fm.horizontalAdvance(m_text);
    int sep = (!m_icon.isNull() && !m_text.isEmpty()) ? m_spacing : 0;
    QMargins m = contentsMargins();

    int w = iconW + sep + textW + m.left() + m.right();
    int h = qMax(m_iconSize.height(), fm.height()) + m.top() + m.bottom();

    return QSize(w, h);
}

QSize IconLabel::minimumSizeHint() const
{
    return sizeHint();
}

void IconLabel::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QRect cr = contentsRect();
    int x = cr.x();
    int centerY = cr.y() + cr.height() / 2;

    if (!m_icon.isNull()) {
        QImage img = m_icon.pixmap(m_icon.actualSize(m_iconSize), isEnabled() ? QIcon::Normal : QIcon::Disabled).toImage();
        if (m_invertIconColors && QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
            img.invertPixels();
        }

        QRect iconRect(x, centerY - m_iconSize.height() / 2, m_iconSize.width(), m_iconSize.height());
        painter.drawImage(iconRect, img);
        x += m_iconSize.width() + m_spacing;
    }

    if (!m_text.isEmpty()) {
        QPalette::ColorGroup cg = isEnabled() ? QPalette::Normal : QPalette::Disabled;
        painter.setPen(palette().color(cg, QPalette::WindowText));
        QRect textRect(x, cr.y(), cr.right() - x, cr.height());
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, m_text);
    }
}

void IconLabel::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::FontChange || e->type() == QEvent::PaletteChange) {
        updateGeometry();
        update();
    }

    QWidget::changeEvent(e);
}
