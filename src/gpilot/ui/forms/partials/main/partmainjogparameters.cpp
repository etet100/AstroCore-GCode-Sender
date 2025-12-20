#include "partmainjogparameters.h"
#include "ui_partmainjogparameters.h"

partMainJogParameters::partMainJogParameters(QWidget* parent)
    : partMainJogParametersInterface(parent)
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

partMainJogParameters::~partMainJogParameters()
{
    delete ui;
}

void partMainJogParameters::updateControls()
{
    // ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    // //ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(m_configuration.uiModule().fontSize()));
    // ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());
}

// partMainJogParameters::partMainJog(QWidget *parent)
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

void partMainJogParameters::configurationUpdated()
{
    // Sep. feed settings for Z axis

    // ui->chkSeparateZFeed->setChecked(m_configurationJogging->separateFeedZ());
    ui->middlePartLayout->setRowVisible(2, m_configurationJogging->separateFeedZ());

    //

    double stepSize = m_configurationJogging->step();
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

void partMainJogParameters::initialize(ConfigurationJogging &configurationJogging)
{
    m_configurationJogging = &configurationJogging;
}

void partMainJogParameters::onCmdFeedChanged(int index)
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

void partMainJogParameters::onCmdFeedZChanged(int index)
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

void partMainJogParameters::onCmdStepChanged(int index)
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

void partMainJogParameters::onChkSeparateZFeedToggled(bool checked)
{
    m_configurationJogging->setSeparateFeedZ(checked);
    ui->middlePartLayout->setRowVisible(2, checked);

    qDebug() << "[Jog UI] Separate Z feed toggled" << checked;
}

void partMainJogParameters::setStepSizeOptions(const QList<float>& options) {
    QStringList items;
    for(float opt : options) {
        items << QString::number(opt);
    }
    ui->cboJogStep->setItems(items);
}

void partMainJogParameters::setFeedRateXYOptions(const QList<float>& options) {
    QStringList items;
    for(float opt : options) {
        items << QString::number(opt);
    }
    ui->cboJogFeed->setItems(items);
}

void partMainJogParameters::setFeedRateZOptions(const QList<float>& options) {
    QStringList items;
    for(float opt : options) {
        items << QString::number(opt);
    }
    ui->cboJogFeedZ->setItems(items);
}

void partMainJogParameters::setStepSize(float value) {
    ui->cboJogStep->setCurrentText(QString::number(value));
}

void partMainJogParameters::setFeedRateXY(float value) {
    ui->cboJogFeed->setCurrentText(QString::number(value));
}

void partMainJogParameters::setFeedRateZ(float value) {
    ui->cboJogFeedZ->setCurrentText(QString::number(value));
}

void partMainJogParameters::setSeparateZFeedrate(bool enabled) {
    ui->middlePartLayout->setRowVisible(2, enabled);
}

bool partMainJogParameters::isSeparateZFeedrate() const {
    return ui->cboJogFeedZ->isVisible();
}

float partMainJogParameters::getStepSize() const {
    return ui->cboJogStep->currentText().toFloat();
}

float partMainJogParameters::getFeedRateXY() const {
    return ui->cboJogFeed->currentText().toFloat();
}

float partMainJogParameters::getFeedRateZ() const {
    return ui->cboJogFeedZ->currentText().toFloat();
}
