#ifndef DLGEDITPROGRAM_H
#define DLGEDITPROGRAM_H

#include <QDialog>
#include "ui/utils/syntaxhighlighter.h"

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
        SyntaxHighlighter* m_highlighter;
};

#endif // DLGEDITPROGRAM_H
