#include "qtoolbuttonwithmenu.h"

#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOptionToolButton>
#include <QStylePainter>

QToolButtonWithMenu::QToolButtonWithMenu(QWidget *parent) : QToolButton(parent)
{
    setMouseTracking(true);
}

QIcon QToolButtonWithMenu::menuIndicatorIcon() const
{
    return m_menuIndicatorIcon;
}

void QToolButtonWithMenu::setMenuIndicatorIcon(const QIcon &icon)
{
    m_menuIndicatorIcon = icon;
    update();
}

double QToolButtonWithMenu::indicatorScaleFactor() const
{
    return m_indicatorScaleFactor;
}

void QToolButtonWithMenu::setIndicatorScaleFactor(double factor)
{
    m_indicatorScaleFactor = factor;
    update();
}

int QToolButtonWithMenu::menuIndicatorMargin() const
{
    return m_menuIndicatorMargin;
}

void QToolButtonWithMenu::setMenuIndicatorMargin(int margin)
{
    m_menuIndicatorMargin = margin;
    update();
}

QMenu *QToolButtonWithMenu::buttonMenu() const
{
    return m_menu;
}

void QToolButtonWithMenu::setButtonMenu(QMenu *menu)
{
    m_menu = menu;
}

QSize QToolButtonWithMenu::sizeHint() const
{
    QSize base = QToolButton::sizeHint();
    if (hasButtonMenu()) {
        base.rwidth() += indicatorSize() * 0.6;
    }

    return base;
}

QSize QToolButtonWithMenu::minimumSizeHint() const
{
    QSize base = QToolButton::minimumSizeHint();
    if (hasButtonMenu()) {
        base.rwidth() += indicatorSize() * 0.6;
    }

    return base;
}

bool QToolButtonWithMenu::hasButtonMenu() const
{
    return m_menu != nullptr
        || QToolButton::menu() != nullptr
        || receivers(SIGNAL(menuRequested())) > 0
        || contextMenuPolicy() == Qt::CustomContextMenu;
}

void QToolButtonWithMenu::paintEvent(QPaintEvent *)
{
    // Draw standard QToolButton using QStylePainter (single painter for everything)
    QStylePainter painter(this);
    QStyleOptionToolButton opt;
    initStyleOption(&opt);

    // Suppress Qt's built-in menu indicator — we draw our own
    opt.features &= ~QStyleOptionToolButton::HasMenu;

    // Draw button frame at full width
    painter.drawPrimitive(QStyle::PE_PanelButtonTool, opt);

    // Draw label (icon/text), with right padding reserved for the indicator if menu is present
    QStyleOptionToolButton labelOpt = opt;
    if (hasButtonMenu()) {
        labelOpt.rect.adjust(0, 0, -indicatorSize() * 0.6, 0);
    }
    painter.drawControl(QStyle::CE_ToolButtonLabel, labelOpt);

    if (!hasButtonMenu()) {
        return;
    }

    // Draw menu indicator overlay
    painter.setRenderHint(QPainter::Antialiasing);

    QRect areaRect = menuIndicatorRect();
    qreal radius = areaRect.height() * 0.2;

    // Semi-transparent background
    QColor bgColor = palette().color(QPalette::ButtonText);
    if (m_menuIndicatorHovered) {
        bgColor.setAlphaF(0.18);
    } else {
        bgColor.setAlphaF(0.10);
    }

    QPainterPath bgPath;
    bgPath.addRoundedRect(QRectF(areaRect), radius, radius);
    painter.fillPath(bgPath, bgColor);

    // Icon or default arrow
    QRect iconRect = areaRect.adjusted(2, 2, -2, -2);

    if (!m_menuIndicatorIcon.isNull()) {
        QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;
        m_menuIndicatorIcon.paint(&painter, iconRect, Qt::AlignCenter, mode);
    } else {
        // Default: down-arrow triangle
        QColor arrowColor = palette().color(
            isEnabled() ? QPalette::Active : QPalette::Disabled,
            QPalette::ButtonText
        );
        if (m_menuIndicatorHovered) {
            arrowColor.setAlphaF(0.9);
        } else {
            arrowColor.setAlphaF(0.6);
        }

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

void QToolButtonWithMenu::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && isMenuIndicatorClick(e->position().toPoint())) {
        m_menuIndicatorPressed = true;
        e->accept();

        return;
    }

    m_menuIndicatorPressed = false;
    QToolButton::mousePressEvent(e);
}

void QToolButtonWithMenu::mouseReleaseEvent(QMouseEvent *e)
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

void QToolButtonWithMenu::mouseMoveEvent(QMouseEvent *e)
{
    bool hovered = isMenuIndicatorClick(e->position().toPoint());
    if (hovered != m_menuIndicatorHovered) {
        m_menuIndicatorHovered = hovered;
        setCursor(hovered ? Qt::ArrowCursor : Qt::PointingHandCursor);
        update();
    }

    QToolButton::mouseMoveEvent(e);
}

void QToolButtonWithMenu::leaveEvent(QEvent *e)
{
    if (m_menuIndicatorHovered) {
        m_menuIndicatorHovered = false;
        unsetCursor();
        update();
    }

    QToolButton::leaveEvent(e);
}

int QToolButtonWithMenu::indicatorSize() const
{
    return qRound(fontMetrics().height() * 0.85 * m_indicatorScaleFactor);
}

QRect QToolButtonWithMenu::menuIndicatorRect() const
{
    int size = indicatorSize();

    return QRect(
        width() - size - m_menuIndicatorMargin,
        height() - size - m_menuIndicatorMargin,
        size,
        size
    );
}

bool QToolButtonWithMenu::isMenuIndicatorClick(const QPoint &pos) const
{
    return hasButtonMenu() && menuIndicatorRect().contains(pos);
}

void QToolButtonWithMenu::contextMenuEvent(QContextMenuEvent *e)
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

void QToolButtonWithMenu::showButtonMenu()
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
