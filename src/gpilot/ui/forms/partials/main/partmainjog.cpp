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

    // QColor backgroundColor = QColor(153, 180, 209);

    // ui->cmdXMinus->setBackColor(backgroundColor);
    // ui->cmdXPlus->setBackColor(backgroundColor);
    // ui->cmdYMinus->setBackColor(backgroundColor);
    // ui->cmdYPlus->setBackColor(backgroundColor);

    connect(ui->jogParameters, &PartMainJogParameters3::stepSizeChanged, this, [this](double val) {
        if (val == PartMainJogParametersInterface::CONTINUOUS) {
            qDebug() << "[UI][PartMainJog] Continuous mode enabled";
            m_configurationJogging->setContinuous(true);
        } else {
            qDebug() << "[UI][PartMainJog] Step size set to" << val;
            m_configurationJogging->setContinuous(false);
            m_configurationJogging->setStep(val);
        }
    });
    connect(ui->jogParameters, &PartMainJogParameters3::feedRateXYChanged, this, [this](double val) {
        qDebug() << "[UI][PartMainJog] Feed rate XY set to" << val;
        m_configurationJogging->setFeed(static_cast<int>(val));
    });
    connect(ui->jogParameters, &PartMainJogParameters3::feedRateZChanged, this, [this](double val) {
        qDebug() << "[UI][PartMainJog] Feed rate Z set to" << val;
        m_configurationJogging->setFeedZ(static_cast<int>(val));
    });
    connect(ui->chkContinuous, &QCheckBox::toggled, this, [this](bool checked) {
        qDebug() << "[UI][PartMainJog] Continuous mode toggled" << checked;
        m_configurationJogging->setContinuous(checked);
    });
    if (ui->jogParameters->handlesContinuous()) {
        ui->chkContinuous->setVisible(false);
    }
}

void PartMainJog::configurationUpdated()
{
    m_storedKeyboardControl = m_configurationJogging->keyboardControl();

    // Sep. feed settings for Z axis

    ui->chkSeparateZFeed->setChecked(m_configurationJogging->separateFeedZ());

    ui->chkContinuous->setChecked(m_configurationJogging->continuous());
    if (ui->jogParameters->handlesContinuous()) {
        ui->jogParameters->setContinuous();
    }

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
    // ui->chkContinuous->setChecked(m_storedKeyboardControl);
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
    m_storedKeyboardControl = ui->chkContinuous->isChecked();
    // ui->chkContinuous->setChecked(false);
}

void PartMainJog::setKeyboardControl(bool value)
{
    ui->chkContinuous->setChecked(value);
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
    if (m_configurationJogging->continuous()) {
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

void PartMainJog::onCmdXMinusYMinusPressed()
{
    m_jogVector = JoggingVector(-1, -1, 0);
    emit this->jog(JoggindDir::XMinusYMinus, m_jogVector);
}

void PartMainJog::onCmdXMinusYMinusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdXMinusYPlusPressed()
{
    m_jogVector = JoggingVector(-1, 1, 0);
    emit this->jog(JoggindDir::XMinusYPlus, m_jogVector);
}

void PartMainJog::onCmdXMinusYPlusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdXPlusYPlusPressed()
{
    m_jogVector = JoggingVector(1, 1, 0);
    emit this->jog(JoggindDir::XPlusYPlus, m_jogVector);
}

void PartMainJog::onCmdXPlusYPlusReleased()
{
    stopJoggingIfContinuous();
}

void PartMainJog::onCmdXPlusYMinusPressed()
{
    m_jogVector = JoggingVector(1, -1, 0);
    emit this->jog(JoggindDir::XPlusYMinus, m_jogVector);
}

void PartMainJog::onCmdXPlusYMinusReleased()
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
    m_jogVector = JoggingVector(0, 0, -1);
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

void PartMainJog::onChkSeparateZFeedToggled(bool checked)
{
    m_configurationJogging->setSeparateFeedZ(checked);
    ui->jogParameters->setSeparateZFeedrate(checked);
    // ui->middlePartLayout->setRowVisible(2, checked);

    qDebug() << "[Jog UI] Separate Z feed toggled" << checked;
}
