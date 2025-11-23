#ifndef WINDOWSTASKBAR_H
#define WINDOWSTASKBAR_H

#ifdef WINDOWS

#include <QWidget>
#include "shobjidl.h"

class WindowsTaskbar
{
    public:
        WindowsTaskbar(QWidget *widget);
        void init();
        void setProgress(int value, int total);

    private:
        QWidget *m_widget;
        ITaskbarList3* m_pTaskbar = nullptr;
        HWND hwnd();
};

#endif

#endif // WINDOWSTASKBAR_H
