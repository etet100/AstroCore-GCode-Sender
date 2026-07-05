#ifndef FRMCLOSINGAPP_H
#define FRMCLOSINGAPP_H

#include <QDialog>

namespace Ui {
class FrmClosingApp;
}

class FrmClosingApp : public QDialog
{
        Q_OBJECT

    public:
        explicit FrmClosingApp(QWidget* parent = nullptr);
        ~FrmClosingApp();

    private:
        Ui::FrmClosingApp* ui;
};

#endif // FRMCLOSINGAPP_H
