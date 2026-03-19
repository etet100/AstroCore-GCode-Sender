#include "partmainjogparameters3.h"
#include "ui_partmainjogparameters3.h"
#include "ui/utils/flowlayout.h"
#include <QPushButton>
#include <functional>

PartMainJogParameters3::PartMainJogParameters3(QWidget* parent)
    : PartMainJogParametersInterface(parent)
    , ui(new Ui::PartMainJogParameters3)
{
    ui->setupUi(this);
}

PartMainJogParameters3::~PartMainJogParameters3()
{
    delete ui;
}

void PartMainJogParameters3::populateButtonGroup(QWidget* container, QList<QPushButton*>& buttons,
    const QStringList& options, std::function<void(float)> onSelected)
{
    // Remove existing buttons and layout
    for (QPushButton* btn : buttons) {
        btn->deleteLater();
    }
    buttons.clear();

    FlowLayout *flowLayout = new FlowLayout();
    flowLayout->setContentsMargins(0, 1, 0, 1);
    flowLayout->setSpacing(2);

    QList<QPushButton*>* buttonsPtr = &buttons;
    for (const QString& opt : options) {
        QPushButton *btn = new QPushButton(opt);
        btn->setCheckable(true);
        btn->setProperty("val", opt.toFloat());
        connect(btn, &QPushButton::clicked, this, [btn, buttonsPtr, onSelected]() {
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

void PartMainJogParameters3::selectButton(QList<QPushButton*>& buttons, float value)
{
    for (QPushButton* btn : buttons) {
        btn->setChecked(btn->property("val").toFloat() == value);
    }
}

void PartMainJogParameters3::setStepSizeOptions(const QStringList& options)
{
    populateButtonGroup(ui->contStep, m_stepButtons, options,
        [this](float v) { emit stepSizeChanged(v); });
}

void PartMainJogParameters3::setFeedRateXYOptions(const QStringList& options)
{
    populateButtonGroup(ui->contFeedXY, m_feedXYButtons, options,
        [this](float v) { emit feedRateXYChanged(v); });
}

void PartMainJogParameters3::setFeedRateZOptions(const QStringList& options)
{
    populateButtonGroup(ui->contFeedZ, m_feedZButtons, options,
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
