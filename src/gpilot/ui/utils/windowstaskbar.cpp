#ifdef WINDOWS

#include "windowstaskbar.h"
#include <windows.h>

WindowsTaskbar::WindowsTaskbar(QWidget *widget) : m_widget(widget)
{
}

void WindowsTaskbar::init()
{
    if (CoCreateInstance(CLSID_TaskbarList, nullptr, CLSCTX_ALL, IID_ITaskbarList3, (void**)&m_pTaskbar) == S_OK) {
        if (m_pTaskbar->HrInit() == S_OK) {
            m_pTaskbar->SetProgressState(hwnd(), TBPF_NORMAL);
        }
        m_pTaskbar->Release();
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

#endif
