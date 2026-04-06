#include "xswitchbuttonwithlabel.h"

#include <QHBoxLayout>

XSwitchButtonWithLabel::XSwitchButtonWithLabel(QWidget *parent) : QFrame(parent)
{
    m_switch = new XSwitchButton(this);
    m_switch->setTextOff("");
    m_switch->setTextOn("");
    connect(m_switch, &XSwitchButton::stateChanged, this, [this](bool checked) {
        emit stateChanged(checked);
    });

    m_label = new QLabel(this);
    connect(m_label, &QLabel::linkActivated, this, [this](const QString &link) {
        emit linkActivated(link);
    });

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(6);
    layout->addWidget(m_switch, 0, Qt::AlignVCenter);
    layout->addWidget(m_label, 0, Qt::AlignVCenter);
    layout->addStretch();

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QString XSwitchButtonWithLabel::text() const
{
    return m_label->text();
}

void XSwitchButtonWithLabel::setText(const QString &text)
{
    m_label->setText(text);
}

bool XSwitchButtonWithLabel::checked() const
{
    return m_switch->checked();
}

void XSwitchButtonWithLabel::setChecked(bool checked)
{
    m_switch->setChecked(checked);
}

int XSwitchButtonWithLabel::spacing() const
{
    return layout()->spacing();
}

void XSwitchButtonWithLabel::setSpacing(int spacing)
{
    layout()->setSpacing(spacing);
}

int XSwitchButtonWithLabel::switchHeight() const
{
    return m_switchHeight;
}

void XSwitchButtonWithLabel::setSwitchHeight(int height)
{
    if (m_switchHeight != height) {
        m_switchHeight = height;
        m_switch->setScaleWithFont(false);
        updateSwitchSize();
    }
}

void XSwitchButtonWithLabel::updateSwitchSize()
{
    int w = m_switchHeight * 2;
    m_switch->setFixedSize(w, m_switchHeight);
}

bool XSwitchButtonWithLabel::scaleSwitchWithFont() const
{
    return m_switch->scaleWithFont();
}

void XSwitchButtonWithLabel::setScaleSwitchWithFont(bool enabled)
{
    m_switch->setScaleWithFont(enabled);
}

double XSwitchButtonWithLabel::fontScaleFactor() const
{
    return m_switch->fontScaleFactor();
}

void XSwitchButtonWithLabel::setFontScaleFactor(double factor)
{
    m_switch->setFontScaleFactor(factor);
}

QColor XSwitchButtonWithLabel::bgColorOn() const { return m_switch->bgColorOn(); }
void XSwitchButtonWithLabel::setBgColorOn(const QColor &color) { m_switch->setBgColorOn(color); }

QColor XSwitchButtonWithLabel::bgColorOff() const { return m_switch->bgColorOff(); }
void XSwitchButtonWithLabel::setBgColorOff(const QColor &color) { m_switch->setBgColorOff(color); }

QColor XSwitchButtonWithLabel::sliderColorOn() const { return m_switch->sliderColorOn(); }
void XSwitchButtonWithLabel::setSliderColorOn(const QColor &color) { m_switch->setSliderColorOn(color); }

QColor XSwitchButtonWithLabel::sliderColorOff() const { return m_switch->sliderColorOff(); }
void XSwitchButtonWithLabel::setSliderColorOff(const QColor &color) { m_switch->setSliderColorOff(color); }

QColor XSwitchButtonWithLabel::textColor() const { return m_switch->textColor(); }
void XSwitchButtonWithLabel::setTextColor(const QColor &color) { m_switch->setTextColor(color); }

QString XSwitchButtonWithLabel::textOn() const { return m_switch->textStrOn(); }
void XSwitchButtonWithLabel::setTextOn(const QString &text) { m_switch->setTextOn(text); }

QString XSwitchButtonWithLabel::textOff() const { return m_switch->textStrOff(); }
void XSwitchButtonWithLabel::setTextOff(const QString &text) { m_switch->setTextOff(text); }

XSwitchButton *XSwitchButtonWithLabel::switchButton() const
{
    return m_switch;
}

QLabel *XSwitchButtonWithLabel::label() const
{
    return m_label;
}

QSize XSwitchButtonWithLabel::sizeHint() const
{
    ensurePolished();

    return layout()->sizeHint().grownBy(contentsMargins());
}

QSize XSwitchButtonWithLabel::minimumSizeHint() const
{
    ensurePolished();

    return layout()->minimumSize().grownBy(contentsMargins());
}
