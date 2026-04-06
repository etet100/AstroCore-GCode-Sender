#ifndef PARTMAINMACROS_H
#define PARTMAINMACROS_H

#include <QWidget>

namespace Ui {
class PartMainMacros;
}

class PartMainMacros : public QWidget
{
        Q_OBJECT

    public:
        explicit PartMainMacros(QWidget* parent = nullptr);
        ~PartMainMacros();

    private:
        Ui::PartMainMacros* ui;
};

#endif // PARTMAINMACROS_H
