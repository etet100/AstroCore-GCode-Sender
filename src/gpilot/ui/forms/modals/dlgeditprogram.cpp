#include "dlgeditprogram.h"
#include "ui_dlgeditprogram.h"
#include <QPushButton>
#include <QMessageBox>
#include "utils/openaimanager.h"

DlgEditProgram::DlgEditProgram(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgEditProgram)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Reset)->setText("Annotate with AI");
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* button) {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole) {
            OpenAIManager& o = OpenAIManager::instance();
            button->setEnabled(false);
            o.annotateProgram(ui->txtProgram->toPlainText(), [this, button](const QString& response) {
                ui->txtProgram->setPlainText(response);
                button->setEnabled(true);
            }, [this, button](const QString& error) {
                QMessageBox::warning(this, "AI Annotation Error", error);
                button->setEnabled(true);
            });
        }
    });

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
