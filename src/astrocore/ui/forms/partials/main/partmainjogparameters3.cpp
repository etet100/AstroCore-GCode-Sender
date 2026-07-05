#include "partmainjogparameters3.h"
#include "ui_partmainjogparameters3.h"
#include "ui/utils/flowlayout.h"
#include "styledtoolbutton.h"
#include <QPushButton>
#include <functional>

PartMainJogParameters3::PartMainJogParameters3(QWidget* parent)
    : AbstractPartMainJogParameters(parent)
    , ui(new Ui::PartMainJogParameters3)
{
    ui->setupUi(this);
}

PartMainJogParameters3::~PartMainJogParameters3()
{
    delete ui;
}

void PartMainJogParameters3::populateButtonGroup(QWidget* container, QList<QToolButton*>& buttons,
    const QStringList& options, bool infOption, std::function<void(float)> onSelected)
{
    // Remove existing buttons and layout
    for (QToolButton* btn : buttons) {
        btn->deleteLater();
    }
    buttons.clear();

    FlowLayout *flowLayout = new FlowLayout();
    flowLayout->setContentsMargins(0, 1, 0, 1);
    flowLayout->setSpacing(2);

    QList<QToolButton*>* buttonsPtr = &buttons;
    if (infOption) {
        StyledToolButton *btn = new StyledToolButton();
        btn->setIcon(QIcon(":/images/infinity.svg"));
        btn->setCheckable(true);
        btn->setProperty("val", CONTINUOUS);
        connect(btn, &QToolButton::clicked, this, [btn, buttonsPtr, onSelected]() {
            for (auto* b : *buttonsPtr) {
                b->setChecked(b == btn);
            }
            onSelected(btn->property("val").toFloat());
        });
        flowLayout->addWidget(btn);
        buttons.append(btn);
    }
    for (const QString& opt : options) {
        QToolButton *btn = new QToolButton();
        btn->setText(opt);
        btn->setCheckable(true);
        btn->setProperty("val", opt.toFloat());
        connect(btn, &QToolButton::clicked, this, [btn, buttonsPtr, onSelected]() {
            for (auto* b : *buttonsPtr) {
                b->setChecked(b == btn);
            }
            onSelected(btn->property("val").toFloat());
        });
        flowLayout->addWidget(btn);
        buttons.append(btn);
    }

    delete container->layout();
    container->setLayout(flowLayout);
}

void PartMainJogParameters3::selectButton(QList<QToolButton*>& buttons, float value)
{
    for (QToolButton* btn : buttons) {
        btn->setChecked(btn->property("val").toFloat() == value);
    }
}

void PartMainJogParameters3::setStepSizeOptions(const QStringList& options)
{
    populateButtonGroup(ui->contStep, m_stepButtons, options, true,
        [this](float v) { emit stepSizeChanged(v); });
}

void PartMainJogParameters3::setFeedRateXYOptions(const QStringList& options)
{
    populateButtonGroup(ui->contFeedXY, m_feedXYButtons, options, false,
        [this](float v) { emit feedRateXYChanged(v); });
}

void PartMainJogParameters3::setFeedRateZOptions(const QStringList& options)
{
    populateButtonGroup(ui->contFeedZ, m_feedZButtons, options, false,
        [this](float v) { emit feedRateZChanged(v); });
}

void PartMainJogParameters3::setStepSize(float value)
{
    selectButton(m_stepButtons, value);
}

void PartMainJogParameters3::setFeedRateXY(float value)
{
    selectButton(m_feedXYButtons, value);
}

void PartMainJogParameters3::setFeedRateZ(float value)
{
    selectButton(m_feedZButtons, value);
}

void PartMainJogParameters3::setSeparateZFeedrate(bool enabled)
{
    ui->lblFeedZ->setVisible(enabled);
    ui->contFeedZ->setVisible(enabled);
}

void PartMainJogParameters3::setContinuous()
{
    for (auto& btn : m_stepButtons) {
        btn->setChecked(btn->property("val").toFloat() == CONTINUOUS);
    }
}

float PartMainJogParameters3::stepSize() const
{
    return m_stepSize;
}

float PartMainJogParameters3::feedRateXY() const
{
    return m_feedRateXY;
}

float PartMainJogParameters3::feedRateZ() const
{
    return m_feedRateZ;
}
