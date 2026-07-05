#ifndef WINDOWSTASKBAR_H
#define WINDOWSTASKBAR_H

#ifdef WINDOWS

#include <QWidget>
#include <QObject>
#include <QPropertyAnimation>
#include "shobjidl.h"

class WindowsTaskbar : public QObject
{
    Q_OBJECT

    public:
        WindowsTaskbar(QWidget *widget);
        ~WindowsTaskbar();
        void init();
        void setProgress(int value, int total);
        void setPaused(bool);
        void hide();
        void show();
    private:
        QWidget *m_widget;
        ITaskbarList3* m_pTaskbar = nullptr;
        HWND hwnd();
};

#endif

#endif // WINDOWSTASKBAR_H
