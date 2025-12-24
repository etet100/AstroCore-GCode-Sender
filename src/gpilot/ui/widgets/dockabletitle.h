#ifndef DOCKABLETITLE_H
#define DOCKABLETITLE_H

#include <QWidget>
#include <QDockWidget>

namespace Ui {
class dockableTitle;
}

class DockableTitle : public QWidget
{
        Q_OBJECT

    public:
        explicit DockableTitle(QWidget* parent = nullptr);
        ~DockableTitle();

    private slots:
        void closeClicked();
        void floatingClicked();

    private:
        Ui::dockableTitle* ui;
        bool m_dark;
        QDockWidget* m_dockWidgetParent;
};

#endif // DOCKABLETITLE_H
