#include "partmainjogparameters.h"
#include "ui_partmainjogparameters.h"

PartMainJogParameters::PartMainJogParameters(QWidget* parent)
    : PartMainJogParametersInterface(parent)
    , ui(new Ui::partMainJogParameters)
{
    ui->setupUi(this);

    connect(ui->cboJogStep, &QComboBox::currentTextChanged, this, [this](const QString &text){
        bool ok;
        float val = text.toFloat(&ok);
        if (ok) emit stepSizeChanged(val);
        else if (text == "Continuous") emit stepSizeChanged(0.0f);

        if (m_initialized) onCmdStepChanged(ui->cboJogStep->currentIndex());
    });

    connect(ui->cboJogFeed, &QComboBox::currentTextChanged, this, [this](const QString &text){
        bool ok;
        float val = text.toFloat(&ok);
        if (ok) emit feedRateXYChanged(val);

        if (m_initialized) onCmdFeedChanged(ui->cboJogFeed->currentIndex());
    });

    connect(ui->cboJogFeedZ, &QComboBox::currentTextChanged, this, [this](const QString &text){
        bool ok;
        float val = text.toFloat(&ok);
        if (ok) emit feedRateZChanged(val);

        if (m_initialized) onCmdFeedZChanged(ui->cboJogFeedZ->currentIndex());
    });
}

PartMainJogParameters::~PartMainJogParameters()
{
    delete ui;
}

void PartMainJogParameters::updateControls()
{
    // ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    // //ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(m_configuration.uiModule().fontSize()));
    // ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());
}

// PartMainJogParameters::partMainJog(QWidget *parent)
//     : QWidget(parent)
//     , ui(new Ui::partMainJog)
// {
//     ui->setupUi(this);

//     QColor backgroundColor = QColor(153, 180, 209);

//     ui->cmdXMinus->setBackColor(backgroundColor);
//     ui->cmdXPlus->setBackColor(backgroundColor);
//     ui->cmdYMinus->setBackColor(backgroundColor);
//     ui->cmdYPlus->setBackColor(backgroundColor);
// }

void PartMainJogParameters::configurationUpdated()
{
    // Sep. feed settings for Z axis

    // ui->chkSeparateZFeed->setChecked(m_configurationJogging->separateFeedZ());
    ui->middlePartLayout->setRowVisible(2, m_configurationJogging->separateFeedZ());

    //

    float stepSize = m_configurationJogging->step();
    int feedRate = m_configurationJogging->feed();
    int feedRateZ = m_configurationJogging->feedZ();

    ui->cboJogStep->setItems(QStringList("Continuous") + m_configurationJogging->stepChoices());
    for (const QString &ch : m_configurationJogging->stepChoices()) {
        if (ch.toDouble() == stepSize) {
            ui->cboJogStep->setCurrentText(ch);
            break;
        }
    }

    ui->cboJogFeed->setItems(m_configurationJogging->feedChoices());
    for (const QString &ch : m_configurationJogging->feedChoices()) {
        if (ch.toInt() == feedRate) {
            ui->cboJogFeed->setCurrentText(ch);
            break;
        }
    }

    ui->cboJogFeedZ->setItems(m_configurationJogging->feedChoices());
    for (const QString &ch : m_configurationJogging->feedChoices()) {
        if (ch.toInt() == feedRateZ) {
            ui->cboJogFeedZ->setCurrentText(ch);
            break;
        }
    }

    m_initialized = true;
}

void PartMainJogParameters::initialize(ConfigurationJogging &configurationJogging)
{
    m_configurationJogging = &configurationJogging;
}

void PartMainJogParameters::onCmdFeedChanged(int index)
{
    if (!m_initialized) {
        return;
    }

    // should not happen in real life, only during initialization (clear old items)
    if (index < 0 || index >= m_configurationJogging->feedChoices().count()) {
        return;
    }

    int feedRate = m_configurationJogging->feedChoices().at(index).toInt();
    m_configurationJogging->setFeed(feedRate);

    qDebug() << "[Jog UI] Feed rate changed" << index << feedRate;

    emit this->parametersChanged(feedRate, m_configurationJogging->step());
}

void PartMainJogParameters::onCmdFeedZChanged(int index)
{
    if (!m_initialized) {
        return;
    }

    // should not happen in real life, only during initialization (clear old items)
    if (index < 0 || index >= m_configurationJogging->feedChoices().count()) {
        return;
    }

    int feedRate = m_configurationJogging->feedChoices().at(index).toInt();
    m_configurationJogging->setFeedZ(feedRate);

    qDebug() << "[Jog UI] Feed rate for Z changed" << index << feedRate;

    emit this->parametersChanged(feedRate, m_configurationJogging->step());
}

void PartMainJogParameters::onCmdStepChanged(int index)
{
    if (!m_initialized) {
        return;
    }

    // should not happen in real life, only during initialization (clear old items)
    if (index < 0 || index >= m_configurationJogging->stepChoices().count()) {
        return;
    }

    double stepSize;
    if (index == 0) {
        stepSize = JoggingContinuous;
    } else {
        stepSize = m_configurationJogging->stepChoices().at(index - 1).toDouble();
    }
    m_configurationJogging->setStep(stepSize);

    qDebug() << "[Jog UI] Step size changed" << index << stepSize;

    emit this->parametersChanged(m_configurationJogging->feed(), stepSize);
}

void PartMainJogParameters::onChkSeparateZFeedToggled(bool checked)
{
    m_configurationJogging->setSeparateFeedZ(checked);
    ui->middlePartLayout->setRowVisible(2, checked);

    qDebug() << "[Jog UI] Separate Z feed toggled" << checked;
}

void PartMainJogParameters::setStepSizeOptions(const QStringList& options) {
    ui->cboJogStep->setItems(options);
}

void PartMainJogParameters::setFeedRateXYOptions(const QStringList& options) {
    ui->cboJogFeed->setItems(options);
}

void PartMainJogParameters::setFeedRateZOptions(const QStringList& options) {
    ui->cboJogFeedZ->setItems(options);
}

void PartMainJogParameters::setStepSize(float value) {
    ui->cboJogStep->setCurrentText(QString::number(value));
}

void PartMainJogParameters::setFeedRateXY(float value) {
    ui->cboJogFeed->setCurrentText(QString::number(value));
}

void PartMainJogParameters::setFeedRateZ(float value) {
    ui->cboJogFeedZ->setCurrentText(QString::number(value));
}

void PartMainJogParameters::setSeparateZFeedrate(bool enabled) {
    ui->middlePartLayout->setRowVisible(2, enabled);
}

// bool PartMainJogParameters::isSeparateZFeedrate() const {
//     return ui->cboJogFeedZ->isVisible();
// }

float PartMainJogParameters::stepSize() const {
    return ui->cboJogStep->currentText().toFloat();
}

float PartMainJogParameters::feedRateXY() const {
    return ui->cboJogFeed->currentText().toFloat();
}

float PartMainJogParameters::feedRateZ() const {
    return ui->cboJogFeedZ->currentText().toFloat();
}
