#ifdef WINDOWS

#include "windowstaskbar.h"
#include <windows.h>

WindowsTaskbar::WindowsTaskbar(QWidget *widget) : QObject(widget), m_widget(widget)
{
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
    if (m_pTaskbar) {
        m_pTaskbar->SetProgressValue(hwnd(), value, total);
    }
}

void WindowsTaskbar::setPaused(bool paused)
{
    if (m_pTaskbar) {
        m_pTaskbar->SetProgressState(hwnd(), paused ? TBPF_PAUSED : TBPF_NORMAL);
    }
}

void WindowsTaskbar::hide()
{
    if (m_pTaskbar) {
        m_pTaskbar->SetProgressState(hwnd(), TBPF_NOPROGRESS);
    }
}

void WindowsTaskbar::show()
{
    if (m_pTaskbar) {
        m_pTaskbar->SetProgressState(hwnd(), TBPF_NORMAL);
    }
}

HWND WindowsTaskbar::hwnd()
{
    return (HWND)m_widget->winId();
}

#endif
