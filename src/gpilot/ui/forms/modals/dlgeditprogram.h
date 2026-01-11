#ifndef DLGEDITPROGRAM_H
#define DLGEDITPROGRAM_H

#include <QDialog>

namespace Ui {
class DlgEditProgram;
}

class DlgEditProgram : public QDialog
{
        Q_OBJECT

    public:
        explicit DlgEditProgram(QWidget* parent = nullptr);
        ~DlgEditProgram();

        QString programText() const;
        void setProgramText(const QString& text);

    private:
        Ui::DlgEditProgram* ui;
};

#endif // DLGEDITPROGRAM_H
