#ifndef FILEDROPOVERLAY_H
#define FILEDROPOVERLAY_H

#include <QWidget>

namespace Ui {
class FileDropOverlay;
}

class FileDropOverlay : public QWidget
{
        Q_OBJECT

    public:
        explicit FileDropOverlay(QWidget* parent = nullptr);
        ~FileDropOverlay();

        void showForbidden();
        void showValid();
        bool valid();

    private:
        Ui::FileDropOverlay* ui;
        bool m_isValid;
};

#endif // FILEDROPOVERLAY_H
