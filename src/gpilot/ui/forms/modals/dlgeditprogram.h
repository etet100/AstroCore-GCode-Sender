#ifndef DLGEDITPROGRAM_H
#define DLGEDITPROGRAM_H

#include <QDialog>
#include "ui/utils/syntaxhighlighter.h"
#include "core/macro/macro.h"

namespace Ui {
class DlgEditProgram;
}

class DlgEditProgram : public QDialog
{
        Q_OBJECT

    public:
        explicit DlgEditProgram(QWidget* parent = nullptr);
        ~DlgEditProgram();

        // Program mode
        QString programText() const;
        void setProgramText(const QString& text);

        // Macro mode
        void setMacro(const Macro& macro);
        void updateMacro(Macro& macro) const;

    private:
        Ui::DlgEditProgram* ui;
        SyntaxHighlighter* m_highlighter;
};

#endif // DLGEDITPROGRAM_H
