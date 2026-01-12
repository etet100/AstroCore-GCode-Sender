#include "dlgeditprogram.h"
#include "ui_dlgeditprogram.h"

DlgEditProgram::DlgEditProgram(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgEditProgram)
{
    ui->setupUi(this);

    m_highlighter = new SyntaxHighlighter(ui->txtProgram->document());
}

DlgEditProgram::~DlgEditProgram()
{
    delete ui;
}

QString DlgEditProgram::programText() const
{
    return ui->txtProgram->toPlainText();
}

void DlgEditProgram::setProgramText(const QString &text)
{
    ui->txtProgram->setPlainText(text);
}
