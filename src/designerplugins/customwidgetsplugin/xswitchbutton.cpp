#include "xswitchbutton.h"
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

XSwitchButton::XSwitchButton(QWidget *parent) : QWidget(parent)
{
    m_space = 2;
    m_radius = 5;
    m_checked = false;
    m_showText = true;
    m_animation = true;

    m_bgColorOn = QColor(0, 180, 255);
    m_bgColorOff = QColor(100, 100, 100);

    m_sliderColorOn= QColor(255, 255, 255);
    m_sliderColorOff = QColor(255, 255, 255);

    m_textColor = QColor(255, 255, 255);

    m_textStrOn = "On";
    m_textStrOff = "Off";

    m_slideStep = 0;
    m_aniStartX = 0;
    m_aniEndX = 0;

    m_aniTimer = new QTimer(this);
    m_aniTimer->setInterval(30);
    connect(m_aniTimer, &QTimer::timeout, this, &XSwitchButton::updateValue);

#ifdef ENABLE_LOADING
    m_loading = true;
    loading_color = m_bgColorOff;
    m_aniCount = 0;

    m_outRadiuslist[0] = 4;
    m_outRadiuslist[1] = 3;
    m_outRadiuslist[2] = 3;
    m_outRadiuslist[3] = 2;
    m_outRadiuslist[4] = 2;
    m_outRadiuslist[5] = 1;
    m_outRadiuslist[6] = 1;
    m_outRadiuslist[7] = 1;
#endif

    if (m_scaleWithFont) {
        updateFixedSizeFromFont();
    }
}

void XSwitchButton::drawBackGround(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor bgColor = m_checked ? m_bgColorOn : m_bgColorOff;
    if ( ! isEnabled()) {
        bgColor.setAlpha(150);
    }

    painter->setBrush(bgColor);

    QRect rect(0, 0, width(), height());
    int side = qMin(width(), height());

    // Left semicircle
    QPainterPath path1;
    // path1.addEllipse(rect.x(), rect.y(), side, side);
    path1.addRoundedRect(rect.x(), rect.y(), side, side, m_radius, m_radius);

    // Right semicircle
    QPainterPath path2;
    // path2.addEllipse(rect.width() - side, rect.y(), side, side);
    path2.addRoundedRect(rect.width() - side, rect.y(), side, side, m_radius, m_radius);

    // Middle rectangle
    QPainterPath path3;
    path3.addRect(rect.x() + side / 2, rect.y(), rect.width() - side, height());

    QPainterPath path = path1 + path2 + path3;
    painter->drawPath(path);

    // Draw text
    // Slider radius
    int sliderWidth = qMin(height(), width()) - m_space * 2 - 5;
    if (m_checked){
        QRect textRect(0, 0, width() - sliderWidth, height());
        painter->setPen(QPen(m_textColor));
        if (showText()) painter->drawText(textRect, Qt::AlignCenter, m_textStrOn);
    } else {
        QRect textRect(sliderWidth, 0, width() - sliderWidth, height());
        painter->setPen(QPen(m_textColor));
        if (showText()) painter->drawText(textRect, Qt::AlignCenter, m_textStrOff);
    }

    painter->restore();
}

void XSwitchButton::drawSlider(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    QColor color = m_checked ? m_sliderColorOn : m_sliderColorOff;

    painter->setBrush(QBrush(color));

    m_sliderWidth = qMin(width(), height()) - m_space * 2;
    QRect rect(m_space + m_aniStartX, m_space, m_sliderWidth, m_sliderWidth);
    // painter->drawEllipse(rect);
    painter->drawRoundedRect(rect, m_radius * 0.8, m_radius * 0.8);

    painter->restore();
}

#ifdef ENABLE_LOADING
void XSwitchButton::startLoading()
{
    // Set as not clickable
    this->setDisabled(true);
    m_displayedWhenStopped = false;
    m_aniCount = 0;
    for (int var = 0; var < 8; var++) {
        loading_point[var].setX(m_aniStartX + m_relXList[var]);
    }
    m_aniTimer->start();
}

void XSwitchButton::stopLoading()
{
    // Set as clickable
    this->setDisabled(false);
    m_aniTimer->stop();
    m_displayedWhenStopped = true;
    update();
}

void XSwitchButton::drawLoading(QPainter *painter)
{
    painter->save();
    painter->setPen(Qt::NoPen);

    for (int var = 0; var < 8; ++var) {
        QPen pen(loading_color); // Define pen
        pen.setWidth(1);
        pen.setStyle(Qt::CustomDashLine);
        painter->setPen(pen);
        painter->setBrush(QBrush(loading_color, Qt::SolidPattern));
        painter->drawEllipse(loading_point[var],m_outRadiuslist[var],m_outRadiuslist[var]);
    }

    painter->restore();
}
#endif

void XSwitchButton::resizeEvent(QResizeEvent *ev)
{
    Q_UNUSED(ev)

    m_sliderWidth = qMin(width(), height()) - m_space * 2;
    m_slideStep = width() / 10;

    if (m_checked) {
        m_aniStartX = width() - height();
        m_aniEndX = m_aniStartX;
    } else {
        m_aniStartX = 0;
        m_aniEndX = 0;
    }

#ifdef ENABLE_LOADING
    m_outRadius = m_sliderWidth/2 + m_space;
    m_inRadius = m_sliderWidth/2 - m_space*2;

    int x_offset = m_sliderWidth/2 + m_space;
    m_relXList[0] = x_offset + m_inRadius;
    m_relXList[1] = x_offset + m_inRadius*3/4;
    m_relXList[2] = x_offset;
    m_relXList[3] = x_offset - m_inRadius*3/4;
    m_relXList[4] = x_offset - m_inRadius;
    m_relXList[5] = x_offset - m_inRadius*3/4;
    m_relXList[6] = x_offset;
    m_relXList[7] = x_offset + m_inRadius*3/4;

    loading_point[0] = QPoint(m_aniStartX + m_relXList[0], m_outRadius); // Left
    loading_point[1] = QPoint(m_aniStartX + m_relXList[1], m_outRadius-m_inRadius*3/4); // Top-left
    loading_point[2] = QPoint(m_aniStartX + m_relXList[2], m_outRadius-m_inRadius); // Top
    loading_point[3] = QPoint(m_aniStartX + m_relXList[3], m_outRadius-m_inRadius*3/4); // Top-right
    loading_point[4] = QPoint(m_aniStartX + m_relXList[4], m_outRadius); // Right
    loading_point[5] = QPoint(m_aniStartX + m_relXList[5], m_outRadius+m_inRadius*3/4); // Bottom-right
    loading_point[6] = QPoint(m_aniStartX + m_relXList[6], m_outRadius+m_inRadius); // Bottom
    loading_point[7] = QPoint(m_aniStartX + m_relXList[7], m_outRadius+m_inRadius*3/4); // Bottom-left
#endif
}

void XSwitchButton::paintEvent(QPaintEvent *ev)
{
    Q_UNUSED(ev)

    // Enable anti-aliasing
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    // Draw background
    drawBackGround(&painter);

    // Draw slider
    drawSlider(&painter);

#ifdef ENABLE_LOADING
    // Draw loading animation
    if(false == m_displayedWhenStopped)
        drawLoading(&painter);
#endif
}

void XSwitchButton::changeEvent(QEvent *ev)
{
    if (ev->type() == QEvent::FontChange && m_scaleWithFont) {
        updateFixedSizeFromFont();
    }

    QWidget::changeEvent(ev);
}

void XSwitchButton::statChanged()
{
    // Calculate step
    m_slideStep = width() / 10;

    // Calculate slider X end coordinate
    if (m_checked) {
        m_aniEndX = width() - height();
    } else {
        m_aniEndX = 0;
    }

    // Check if animation is used
    if (m_animation) {
        m_aniTimer->start();
    } else{
        m_aniStartX = m_aniEndX;
        update();
    }

    if (c_stateChangedCallback) c_stateChangedCallback(m_checked);
    emit stateChanged(m_checked);
}

void XSwitchButton::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() != Qt::MouseButton::LeftButton) return;

    //Q_UNUSED(ev)
#ifdef ENABLE_LOADING
    if (m_loading) {
        startLoading();
    }
    else
#endif
    {
        m_checked = !m_checked;
        statChanged();
    }
}

void XSwitchButton::updateValue()
{
#ifdef ENABLE_LOADING
    if(false == m_displayedWhenStopped){
        if(0 != ++m_aniCount%3) return;
        m_aniCount = 0;
        int tmpInt = m_outRadiuslist[0];
        for (int var = 0; var < 7; var++) {
            m_outRadiuslist[var] = m_outRadiuslist[var+1];
        }
        m_outRadiuslist[7] = tmpInt;
    }else
#endif
    {
        if (m_checked) {
            if (m_aniStartX < m_aniEndX) {
                m_aniStartX += m_slideStep;
            } else {
                m_aniStartX = m_aniEndX;
                m_aniTimer->stop();
            }
        } else {
            if (m_aniStartX > m_aniEndX) {
                m_aniStartX -= m_slideStep;
            } else {
                m_aniStartX = m_aniEndX;
                m_aniTimer->stop();
            }
        }
    }

    update();
}

QSize XSwitchButton::sizeHint() const
{
    int h;
    if (m_scaleWithFont) {
        QFontMetrics fm(font());
        h = qRound(fm.height() * m_fontScaleFactor);
    } else {
        h = 20;
    }
    int w = h * 2;

    return QSize(w, h);
}

QSize XSwitchButton::minimumSizeHint() const
{
    return sizeHint();
}

int XSwitchButton::space() const
{
    return m_space;
}

int XSwitchButton::radius() const
{
    return m_radius;
}

bool XSwitchButton::checked() const
{
    return m_checked;
}

bool XSwitchButton::showText() const
{
    return m_showText;
}

bool XSwitchButton::animation() const
{
    return m_animation;
}

QColor XSwitchButton::bgColorOn() const
{
    return m_bgColorOn;
}

QColor XSwitchButton::bgColorOff() const
{
    return m_bgColorOff;
}

QColor XSwitchButton::sliderColorOn() const
{
    return m_sliderColorOn;
}

QColor XSwitchButton::sliderColorOff() const
{
    return m_sliderColorOff;
}

QColor XSwitchButton::textColor() const
{
    return m_textColor;
}

QString XSwitchButton::textStrOn() const
{
    return m_textStrOn;
}

QString XSwitchButton::textStrOff() const
{
    return m_textStrOff;
}

int XSwitchButton::slideStep() const
{
    return m_slideStep;
}

int XSwitchButton::aniStartX() const
{
    return m_aniStartX;
}

int XSwitchButton::aniEndX() const
{
    return m_aniEndX;
}

void XSwitchButton::setSpace(int space)
{
    if (m_space != space) {
        m_space = space;
        update();
    }
}

void XSwitchButton::setRadius(int radius)
{
    if (m_radius != radius) {
        m_radius = radius;
        update();
    }
}

void XSwitchButton::setChecked(bool checked)
{
    if (m_checked != checked) {
        m_checked = checked;
        update();
    }

#ifdef ENABLE_LOADING
    stopLoading();
#endif
    statChanged();
}

#ifdef ENABLE_LOADING
bool XSwitchButton::loading(){
    return m_loading;
}

void XSwitchButton::setLoading(bool enabled){
    m_loading = enabled;
}
#endif

void XSwitchButton::setShowText(bool show)
{
    if (m_showText != show) {
        m_showText = show;
        update();
    }
}

void XSwitchButton::setAnimation(bool ok)
{
    if (m_animation != ok) {
        m_animation = ok;
        update();
    }
}

void XSwitchButton::setBgColorOn(const QColor &color)
{
    if (m_bgColorOn != color) {
        m_bgColorOn = color;
        update();
    }
}

void XSwitchButton::setBgColorOff(const QColor &color)
{
    if (m_bgColorOff != color) {
        m_bgColorOff = color;
        update();
    }
}

void XSwitchButton::setSliderColorOn(const QColor &color)
{
    if (m_sliderColorOn != color) {
        m_sliderColorOn = color;
        update();
    }
}

void XSwitchButton::setSliderColorOff(const QColor &color)
{
    if (m_sliderColorOff != color) {
        m_sliderColorOff = color;
        update();
    }
}

void XSwitchButton::setTextColor(const QColor &color)
{
    if (m_textColor != color) {
        m_textColor = color;
        update();
    }
}

void XSwitchButton::setTextOn(const QString &text)
{
    if (m_textStrOn != text) {
        m_textStrOn = text;
        update();
    }
}

void XSwitchButton::setTextOff(const QString &text)
{
    if (m_textStrOff != text) {
        m_textStrOff = text;
        update();
    }
}

bool XSwitchButton::scaleWithFont() const
{
    return m_scaleWithFont;
}

void XSwitchButton::setScaleWithFont(bool enabled)
{
    if (m_scaleWithFont != enabled) {
        m_scaleWithFont = enabled;
        if (m_scaleWithFont) {
            updateFixedSizeFromFont();
        } else {
            setMinimumSize(0, 0);
            setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
        }
    }
}

double XSwitchButton::fontScaleFactor() const
{
    return m_fontScaleFactor;
}

void XSwitchButton::setFontScaleFactor(double factor)
{
    if (!qFuzzyCompare(m_fontScaleFactor, factor)) {
        m_fontScaleFactor = factor;
        if (m_scaleWithFont) {
            updateFixedSizeFromFont();
        }
    }
}

void XSwitchButton::updateFixedSizeFromFont()
{
    QFontMetrics fm(font());
    int h = qRound(fm.height() * m_fontScaleFactor);
    int w = h * 2;
    setFixedSize(w, h);
}
