#include "partmainjog.h"
#include "ui_partmainjog.h"

void partMainJog::updateControls()
{
    ui->cboJogStep->setEditable(!ui->chkKeyboardControl->isChecked());
    ui->cboJogFeed->setEditable(!ui->chkKeyboardControl->isChecked());
    ui->cboJogStep->setEnabled(!ui->chkKeyboardControl->isChecked());
    ui->cboJogFeed->setEnabled(!ui->chkKeyboardControl->isChecked());
    //ui->cboJogStep->setStyleSheet(QString("font-size: %1").arg(m_configuration.uiModule().fontSize()));
    ui->cboJogFeed->setStyleSheet(ui->cboJogStep->styleSheet());
}

partMainJog::partMainJog(QWidget *parent)
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

void partMainJog::configurationUpdated()
{
    m_storedKeyboardControl = m_configurationJogging->keyboardControl();

    ui->cboJogStep->setItems(QStringList("Continuous") + m_configurationJogging->stepChoices());
    for (QString ch : m_configurationJogging->stepChoices()) {
        if (ch.toDouble() == m_configurationJogging->jogStep()) {
            ui->cboJogStep->setCurrentText(ch);
            break;
        }
    }

    ui->cboJogFeed->setItems(m_configurationJogging->feedChoices());
    for (QString ch : m_configurationJogging->feedChoices()) {
        if (ch.toInt() == m_configurationJogging->jogFeed()) {
            ui->cboJogFeed->setCurrentText(ch);
            break;
        }
    }

    m_initialized = true;
}

void partMainJog::restoreKeyboardControl()
{
    ui->chkKeyboardControl->setChecked(m_storedKeyboardControl);
}

void partMainJog::initialize(ConfigurationJogging &configurationJogging)
{
    m_configurationJogging = &configurationJogging;
}

partMainJog::~partMainJog()
{
    delete ui;
}

void partMainJog::storeAndResetKeyboardControl()
{
    m_storedKeyboardControl = ui->chkKeyboardControl->isChecked();
    ui->chkKeyboardControl->setChecked(false);
}

bool partMainJog::keyboardControl()
{
    return ui->chkKeyboardControl->isChecked();
}

void partMainJog::setKeyboardControl(bool value)
{
    ui->chkKeyboardControl->setChecked(value);
}

void partMainJog::onCmdYPlusPressed()
{
    m_jogVector = JoggingVector(0, 1, 0);
    emit this->jog(JoggindDir::YPlus, m_jogVector);
}

void partMainJog::stopJogging()
{
    m_jogVector = QVector3D(0, 0, 0);
    emit stop();
    emit this->jog(JoggindDir::None, m_jogVector);
    emit this->command(GRBLCommand::JogStop);
}

void partMainJog::stopJoggingIfContinuous()
{
    if (m_stepSize == JoggingContinuous) {
        stopJogging();
    }
}

void partMainJog::onCmdYPlusReleased()
{
    stopJoggingIfContinuous();
}

void partMainJog::onCmdYMinusPressed()
{
    m_jogVector = JoggingVector(0, -1, 0);
    emit this->jog(JoggindDir::YMinus, m_jogVector);
}

void partMainJog::onCmdYMinusReleased()
{
    stopJoggingIfContinuous();
}

void partMainJog::onCmdXPlusPressed()
{
    m_jogVector = JoggingVector(1, 0, 0);
    emit this->jog(JoggindDir::XPlus, m_jogVector);
}

void partMainJog::onCmdXPlusReleased()
{
    stopJoggingIfContinuous();
}

void partMainJog::onCmdXMinusPressed()
{
    m_jogVector = JoggingVector(-1, 0, 0);
    emit this->jog(JoggindDir::XMinus, m_jogVector);
}

void partMainJog::onCmdXMinusReleased()
{
    stopJoggingIfContinuous();
}

void partMainJog::onCmdZPlusPressed()
{
    m_jogVector = JoggingVector(0, 0, 1);
    emit this->jog(JoggindDir::ZPlus, m_jogVector);
}

void partMainJog::onCmdZPlusReleased()
{
    stopJoggingIfContinuous();
}

void partMainJog::onCmdZMinusPressed()
{
    m_jogVector = JoggingVector(-1, 0, 0);
    emit this->jog(JoggindDir::ZMinus, m_jogVector);
}

void partMainJog::onCmdZMinusReleased()
{
    stopJoggingIfContinuous();
}

void partMainJog::onCmdStopClicked()
{
    stopJogging();
}

void partMainJog::onCmdFeedRateChanged(int index)
{
    if (!m_initialized) {
        return;
    }

    // should not happen in real life, only during initialization (clear old items)
    if (index < 0 || index >= m_configurationJogging->feedChoices().count()) {
        return;
    }

    m_feedRate = m_configurationJogging->feedChoices().at(index).toInt();
    m_configurationJogging->setJogFeed(m_feedRate);

    qDebug() << "[Jog UI] Feed rate changed" << index << m_feedRate;

    emit this->parametersChanged(m_feedRate, m_stepSize);
}

void partMainJog::onCmdStepSizeChanged(int index)
{
    if (!m_initialized) {
        return;
    }

    // should not happen in real life, only during initialization (clear old items)
    if (index < 0 || index >= m_configurationJogging->stepChoices().count()) {
        return;
    }

    if (index == 0) {
        m_stepSize = JoggingContinuous;
    } else {
        m_stepSize = m_configurationJogging->stepChoices().at(index - 1).toDouble();
    }
    m_configurationJogging->setJogStep(m_stepSize);

    qDebug() << "[Jog UI] Step size changed" << index << m_stepSize;

    emit this->parametersChanged(m_feedRate, m_stepSize);
}
