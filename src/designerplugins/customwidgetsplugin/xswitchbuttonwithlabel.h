#ifndef XSWITCHBUTTONWITHLABEL_H
#define XSWITCHBUTTONWITHLABEL_H

#include <QFrame>
#include <QLabel>
#include "xswitchbutton.h"
#include "customwidgetsshared.h"

class CUSTOMWIDGETS_DLLSPEC XSwitchButtonWithLabel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool checked READ checked WRITE setChecked)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(int switchHeight READ switchHeight WRITE setSwitchHeight)
    Q_PROPERTY(QColor bgColorOn READ bgColorOn WRITE setBgColorOn)
    Q_PROPERTY(QColor bgColorOff READ bgColorOff WRITE setBgColorOff)
    Q_PROPERTY(QColor sliderColorOn READ sliderColorOn WRITE setSliderColorOn)
    Q_PROPERTY(QColor sliderColorOff READ sliderColorOff WRITE setSliderColorOff)
    Q_PROPERTY(QColor textColor READ textColor WRITE setTextColor)
    Q_PROPERTY(QString textOn READ textOn WRITE setTextOn)
    Q_PROPERTY(QString textOff READ textOff WRITE setTextOff)
    Q_PROPERTY(bool scaleSwitchWithFont READ scaleSwitchWithFont WRITE setScaleSwitchWithFont)
    Q_PROPERTY(double fontScaleFactor READ fontScaleFactor WRITE setFontScaleFactor)

public:
    explicit XSwitchButtonWithLabel(QWidget *parent = nullptr);

    QString text() const;
    void setText(const QString &text);

    bool checked() const;
    void setChecked(bool checked);

    int spacing() const;
    void setSpacing(int spacing);

    int switchHeight() const;
    void setSwitchHeight(int height);

    QColor bgColorOn() const;
    void setBgColorOn(const QColor &color);
    QColor bgColorOff() const;
    void setBgColorOff(const QColor &color);
    QColor sliderColorOn() const;
    void setSliderColorOn(const QColor &color);
    QColor sliderColorOff() const;
    void setSliderColorOff(const QColor &color);
    QColor textColor() const;
    void setTextColor(const QColor &color);
    QString textOn() const;
    void setTextOn(const QString &text);
    QString textOff() const;
    void setTextOff(const QString &text);

    bool scaleSwitchWithFont() const;
    void setScaleSwitchWithFont(bool enabled);
    double fontScaleFactor() const;
    void setFontScaleFactor(double factor);

    XSwitchButton *switchButton() const;
    QLabel *label() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void stateChanged(bool checked);
    void linkActivated(const QString& link);

private:
    void updateSwitchSize();

    XSwitchButton *m_switch;
    QLabel *m_label;
    int m_switchHeight = 20;
};

#endif // XSWITCHBUTTONWITHLABEL_H
