#ifndef DOCKABLETITLE_H
#define DOCKABLETITLE_H

#include <QWidget>

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
};

#endif // DOCKABLETITLE_H
