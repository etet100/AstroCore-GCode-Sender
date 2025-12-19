#ifndef XSWITCHBUTTON_H
#define XSWITCHBUTTON_H

#include <QWidget>
#include <QTimer>
#include <QColor>
#include <QDebug>
#include "customwidgetsshared.h"

// #define ENABLE_LOADING 1

class CUSTOMWIDGETS_DLLSPEC XSwitchButton : public QWidget
{
    Q_OBJECT

public:
    explicit XSwitchButton(QWidget *parent = 0);
    ~XSwitchButton() {}
    void setStateChangedCallback(std::function<void(bool)> cb) { c_stateChangedCallback = cb; }

private slots:
    void updateValue();

private:
    Q_DISABLE_COPY(XSwitchButton)
    void drawBackGround(QPainter *painter);
    void drawSlider(QPainter *painter);

protected:
    void resizeEvent(QResizeEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void statChanged();

private:
    int m_space;                // Distance from slider to edge
    int m_radius;               // Corner radius

    bool m_checked;             // Checked state
    bool m_showText;            // Show text
    bool m_animation;           // Use animation

    std::function<void(bool)> c_stateChangedCallback = nullptr;
#ifdef ENABLE_LOADING
    bool m_loading;             // Use loading animation

    QColor loading_color;
    QPoint loading_point[8];
    int m_inRadius;
    int m_outRadius;
    int m_outRadiuslist[8];
    int m_relXList[8];
    bool m_displayedWhenStopped = true; // Loading animation running flag
    int m_aniCount;              // Animation timer count, controls loading frame rate

    void startLoading();
    void stopLoading();
    void drawLoading(QPainter *painter);
#endif

    QColor m_bgColorOn;         // Background color when ON
    QColor m_bgColorOff;        // Background color when OFF
    QColor m_sliderColorOn;     // Slider color when ON
    QColor m_sliderColorOff;    // Slider color when OFF
    QColor m_textColor;          // Text color

    QString m_textStrOn;        // Text when ON
    QString m_textStrOff;       // Text when OFF

    QTimer  *m_aniTimer;         // Animation timer
    int     m_slideStep;         // Animation step
    int     m_sliderWidth;       // Slider width
    int     m_aniStartX;         // Slider start X coordinate
    int     m_aniEndX;           // Slider end X coordinate

public:
    int space() const;
    int radius() const;
    bool checked() const;
    bool showText() const;
    bool animation() const;
#ifdef ENABLE_LOADING
    bool loading();
#endif

    QColor bgColorOn() const;
    QColor bgColorOff() const;
    QColor sliderColorOn() const;
    QColor sliderColorOff() const;
    QColor textColor() const;

    QString textStrOn() const;
    QString textStrOff() const;

    int slideStep() const;
    int aniStartX() const;
    int aniEndX() const;

public Q_SLOTS:
    void setSpace(int space);
    void setRadius(int radius);
    void setChecked(bool checked);
    void setShowText(bool show);
    void setAnimation(bool ok);
#ifdef ENABLE_LOADING
    void setLoading(bool enabled);
#endif

    void setBgColorOn(const QColor &color);
    void setBgColorOff(const QColor &color);
    void setSliderColorOn(const QColor &color);
    void setSliderColorOff(const QColor &color);
    void setTextColor(const QColor &color);

    void setTextOn(const QString &text);
    void setTextOff(const QString &text);

//    void setStep(int step);
//    void setStartX(int startX);
//    void setEndX(int endX);


};

#endif // XSWITCHBUTTON_H
