#ifndef PARTMAINMACROS_H
#define PARTMAINMACROS_H

#include <QWidget>
#include "core/macro/macros.h"
#include "ui/utils/flowlayout.h"

namespace Ui {
class PartMainMacros;
}

class PartMainMacros : public QWidget
{
        Q_OBJECT

    public:
        explicit PartMainMacros(QWidget* parent = nullptr);
        ~PartMainMacros();
        void updateMacros(const Macros& macros);

    signals:
        void editMacro(int id);
        void runMacro(int id);
        void newMacroRequested();

    private:
        Ui::PartMainMacros* ui;
        FlowLayout* m_flowLayout = nullptr;
};

#endif // PARTMAINMACROS_H
