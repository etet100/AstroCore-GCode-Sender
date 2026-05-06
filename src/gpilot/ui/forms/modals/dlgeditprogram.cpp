#include "dlgeditprogram.h"
#include "ui_dlgeditprogram.h"
#include <QPushButton>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include "modules/ai/openaimanager.h"

static QString macroTypeToString(MacroType type)
{
    switch (type) {
        case MacroType::ProgramStart:     return "Program Start";
        case MacroType::ProgramEnd:       return "Program End";
        case MacroType::BeforePause:      return "Before Pause";
        case MacroType::AfterPause:       return "After Pause";
        case MacroType::BeforeToolChange: return "Before Tool Change";
        case MacroType::AfterToolChange:  return "After Tool Change";
        case MacroType::Custom:           return "Custom";
    }
    return {};
}

DlgEditProgram::DlgEditProgram(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DlgEditProgram)
{
    ui->setupUi(this);
    ui->widget->hide();
    setWindowTitle("Edit Program");

    ui->buttonBox->button(QDialogButtonBox::Reset)->setText("Annotate with AI");
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, [this](QAbstractButton* button) {
        if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole) {
            OpenAIManager& o = OpenAIManager::instance();
            button->setEnabled(false);
            o.annotateProgram(ui->txtProgram->toPlainText(), [this, button](const QString& response) {
                // Comments format: {\"l\": 4, \"d\": \"Move the Z axis to machine home position.\"}
                QStringList comments = response.split('\n');
                QStringList lines = ui->txtProgram->toPlainText().split('\n');

                for (auto& comment : comments) {
                    QJsonObject obj = QJsonDocument::fromJson(comment.toUtf8()).object();
                    int lineNumber = obj["l"].toInt(-1);
                    QString description = obj["d"].toString("").trimmed();
                    if (lineNumber >= lines.size() || description.isEmpty()) {
                        continue;
                    }

                    // Remove existing comment
                    QString line = lines[lineNumber];
                    int commentIndex = line.indexOf(';');
                    if (commentIndex != -1) {
                        line = line.left(commentIndex).trimmed();
                    }
                    line += " ; " + description;
                    lines[lineNumber] = line;
                }

                ui->txtProgram->setPlainText(lines.join('\n'));
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

void DlgEditProgram::setMacro(const Macro& macro)
{
    ui->widget->show();
    ui->lblType->setText(macroTypeToString(macro.type));
    ui->txtName->setText(macro.name);
    ui->chkEnabled->setChecked(macro.enabled);
    ui->txtProgram->setPlainText(macro.content);
    setWindowTitle("Edit Macro");
}

void DlgEditProgram::updateMacro(Macro& macro) const
{
    macro.name = ui->txtName->text();
    macro.enabled = ui->chkEnabled->isChecked();
    macro.content = ui->txtProgram->toPlainText();
}
