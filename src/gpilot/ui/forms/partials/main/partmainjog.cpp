#include "partmainjog.h"
#include "ui_partmainjog.h"

void PartMainJog::updateControls()
{
    // ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    // ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    // //ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(m_configuration.uiModule().fontSize()));
    // ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());
}

PartMainJog::PartMainJog(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::partMainJog)
{
    ui->setupUi(this);

    QColor backgroundColor = QColor(153, 180, 209);

    ui->cmdXMinus->setBackColor(backgroundColor);
    ui->cmdXPlus->setBackColor(backgroundColor);
    ui->cmdYMinus->setBackColor(backgroundColor);
    ui->cmdYPlus->setBackColor(backgroundColor);
}

void PartMainJog::configurationUpdated()
{
    m_storedKeyboardControl = m_configurationJogging->keyboardControl();

    // Sep. feed settings for Z axis

    ui->chkSeparateZFeed->setChecked(m_configurationJogging->separateFeedZ());
    // ui->middlePartLayout->setRowVisible(2, m_configurationJogging->separateFeedZ());

    //

    // double stepSize = m_configurationJogging->step();
    // int feedRate = m_configurationJogging->feed();
    // int feedRateZ = m_configurationJogging->feedZ();

    // ui->cboJogStep->setItems(QStringList("Continuous") + m_configurationJogging->stepChoices());
    // for (const QString &ch : m_configurationJogging->stepChoices()) {
    //     if (ch.toDouble() == stepSize) {
    //         ui->cboJogStep->setCurrentText(ch);
    //         break;
    //     }
    // }

    // ui->cboJogFeed->setItems(m_configurationJogging->feedChoices());
    // for (const QString &ch : m_configurationJogging->feedChoices()) {
    //     if (ch.toInt() == feedRate) {
    //         ui->cboJogFeed->setCurrentText(ch);
    //         break;
    //     }
    // }

    // ui->cboJogFeedZ->setItems(m_configurationJogging->feedChoices());
    // for (const QString &ch : m_configurationJogging->feedChoices()) {
    //     if (ch.toInt() == feedRateZ) {
    //         ui->cboJogFeedZ->setCurrentText(ch);
    //         break;
    //     }
    // }

    ui->jogParameters->setFeedRateXYOptions(m_configurationJogging->feedChoices());
    ui->jogParameters->setFeedRateXY(m_configurationJogging->feed());
    ui->jogParameters->setFeedRateZOptions(m_configurationJogging->feedChoices());
    ui->jogParameters->setFeedRateZ(m_configurationJogging->feedZ());
    ui->jogParameters->setStepSizeOptions(m_configurationJogging->stepChoices());
    ui->jogParameters->setStepSize(m_configurationJogging->step());
    // It has to be calles after FeedRateZOptions is set
    ui->jogParameters->setSeparateZFeedrate(m_configurationJogging->separateFeedZ());

    m_initialized = true;
}

void PartMainJog::restoreKeyboardControl()
{
    ui->chkKeyboardControl->setChecked(m_storedKeyboardControl);
}

void PartMainJog::initialize(ConfigurationJogging &configurationJogging)
{
    m_configurationJogging = &configurationJogging;
}

PartMainJog::~PartMainJog()
{
    delete ui;
}

void PartMainJog::storeAndResetKeyboardControl()
{
    m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
    ui->chkKeyboardControl->setChecked(false);
}

bool PartMainJog::keyboardControl()
{
    return ui->chkKeyboardControl->isChecked();
}

void PartMainJog::setKeyboardControl(bool value)
{
    ui->chkKeyboardControl->setChecked(value);
}

void PartMainJog::onCmdYPlusPressed()
{
    m_jogVector = JoggingVector(0, 1, 0);
    emit this->jog(JoggindDir::YPlus, m_jogVector);
}

void PartMainJog::stopJogging()
{
    m_jogVector = QVector3D(0, 0, 0);
    emit stop();
    emit this->jog(JoggindDir::None, m_jogVector);
    emit this->command(GRBLCommand::JogStop);
}

void PartMainJog::stopJoggingIfContinuous()
{
    if (m_configurationJogging->feedZ() == JoggingContinuous) {
        stopJogging();
    }
}

void PartMainJog::onCmdYPlusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdYMinusPressed()
{
    m_jogVector = JoggingVector(0, -1, 0);
    emit this->jog(JoggindDir::YMinus, m_jogVector);
}

void PartMainJog::onCmdYMinusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdXPlusPressed()
{
    m_jogVector = JoggingVector(1, 0, 0);
    emit this->jog(JoggindDir::XPlus, m_jogVector);
}

void PartMainJog::onCmdXPlusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdXMinusPressed()
{
    m_jogVector = JoggingVector(-1, 0, 0);
    emit this->jog(JoggindDir::XMinus, m_jogVector);
}

void PartMainJog::onCmdXMinusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdZPlusPressed()
{
    m_jogVector = JoggingVector(0, 0, 1);
    emit this->jog(JoggindDir::ZPlus, m_jogVector);
}

void PartMainJog::onCmdZPlusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdZMinusPressed()
{
    m_jogVector = JoggingVector(-1, 0, 0);
    emit this->jog(JoggindDir::ZMinus, m_jogVector);
}

void PartMainJog::onCmdZMinusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdStopClicked()
{
    stopJogging();
}

// void PartMainJog::onCmdFeedChanged(int index)
// {
//     if (!m_initialized) {
//         return;
//     }

//     // should not happen in real life, only during initialization (clear old items)
//     if (index < 0 || index >= m_configurationJogging->feedChoices().count()) {
//         return;
//     }

//     int feedRate = m_configurationJogging->feedChoices().at(index).toInt();
//     m_configurationJogging->setFeed(feedRate);

//     qDebug() << "[Jog UI] Feed rate changed" << index << feedRate;

//     emit this->parametersChanged(feedRate, m_configurationJogging->step());
// }

// void PartMainJog::onCmdFeedZChanged(int index)
// {
//     if (!m_initialized) {
//         return;
//     }

//     // should not happen in real life, only during initialization (clear old items)
//     if (index < 0 || index >= m_configurationJogging->feedChoices().count()) {
//         return;
//     }

//     int feedRate = m_configurationJogging->feedChoices().at(index).toInt();
//     m_configurationJogging->setFeedZ(feedRate);

//     qDebug() << "[Jog UI] Feed rate for Z changed" << index << feedRate;

//     emit this->parametersChanged(feedRate, m_configurationJogging->step());
// }

// void PartMainJog::onCmdStepChanged(int index)
// {
//     if (!m_initialized) {
//         return;
//     }

//     // should not happen in real life, only during initialization (clear old items)
//     if (index < 0 || index >= m_configurationJogging->stepChoices().count()) {
//         return;
//     }

//     double stepSize;
//     if (index == 0) {
//         stepSize = JoggingContinuous;
//     } else {
//         stepSize = m_configurationJogging->stepChoices().at(index - 1).toDouble();
//     }
//     m_configurationJogging->setStep(stepSize);

//     qDebug() << "[Jog UI] Step size changed" << index << stepSize;

//     emit this->parametersChanged(m_configurationJogging->feed(), stepSize);
// }

void PartMainJog::onChkSeparateZFeedToggled(bool checked)
{
    m_configurationJogging->setSeparateFeedZ(checked);
    ui->jogParameters->setSeparateZFeedrate(checked);
    // ui->middlePartLayout->setRowVisible(2, checked);

    qDebug() << "[Jog UI] Separate Z feed toggled" << checked;
}
