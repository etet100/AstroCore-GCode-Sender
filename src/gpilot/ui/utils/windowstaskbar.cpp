#ifdef WINDOWS

#include "windowstaskbar.h"
#include <windows.h>

WindowsTaskbar::WindowsTaskbar(QWidget *widget) : QObject(widget), m_widget(widget)
{
    startAnimator();
}

WindowsTaskbar::~WindowsTaskbar() {
    if (m_pTaskbar) {
        m_pTaskbar->Release();
        m_pTaskbar = nullptr;
    }
}

void WindowsTaskbar::init()
{
    if (CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&m_pTaskbar) == S_OK) {
        if (m_pTaskbar->HrInit() == S_OK) {
            m_pTaskbar->SetProgressState(hwnd(), TBPF_NORMAL);
        }
    }
}

void WindowsTaskbar::setProgress(int value, int total)
{
    m_pTaskbar->SetProgressValue(hwnd(), value, total);
}

HWND WindowsTaskbar::hwnd()
{
    return (HWND)m_widget->winId();
}

void WindowsTaskbar::startAnimator()
{
    m_animator = new QPropertyAnimation(this, "animation");
    m_animator->setDuration(2500);
    m_animator->setStartValue(0);
    m_animator->setEndValue(1);
    m_animator->setEasingCurve(QEasingCurve::InOutSine);
    QObject::connect(m_animator, &QPropertyAnimation::finished, [this]() {
        if (m_animator->direction() == QAbstractAnimation::Forward)
            m_animator->setDirection(QAbstractAnimation::Backward);
        else
            m_animator->setDirection(QAbstractAnimation::Forward);
        m_animator->start();
    });
    m_animator->start();
}

void WindowsTaskbar::setAnimation(float value)
{
    if (m_pTaskbar) {
        setProgress(value * 500, 500);
    }
}

#endif
