#include "partmainjogparameters2.h"
#include "ui_partmainjogparameters2.h"
#include <QStyle>
#include <QGridLayout>
#include <QDebug>
#include <QHBoxLayout>

partMainJogParameters2::partMainJogParameters2(QWidget *parent) : partMainJogParametersInterface(parent), ui(new Ui::partMainJogParameters2)
{
    ui->setupUi(this);
    ui->sectionFrame->deleteLater();

    setStepSizeOptions({0.1, 0.5, 1, 5, 10, 50, 100});
    setFeedRateXYOptions({100, 200, 500, 1000, 2000, 5000});
    setFeedRateZOptions({100, 200, 500, 1000, 2000, 5000});

    // Default visibility
    setSeparateZFeedrate(false);

    style()->unpolish(this);
    style()->polish(this);
}

void partMainJogParameters2::setStepSizeOptions(const QList<float>& options) {
    // max value, multiplier
    QMap<float, float> groups = {
        {1.0f, 0.1f},
        {10.0f, 1.0f},
        {100.0f, 10.0f},
        {1000.0f, 100.0f}
    };


    rebuildSection(m_stepSection, "STEP SIZE", options, groups);
}

void partMainJogParameters2::setFeedRateXYOptions(const QList<float>& options) {
    // max value, multiplier
    QMap<float, float> groups = {
        {10, 1},
        {100, 10},
        {1000, 100}
    };

    rebuildSection(m_feedXYSection, "FEEDRATE (XY)", options, groups);
}

void partMainJogParameters2::setFeedRateZOptions(const QList<float>& options) {
    // max value, multiplier
    QMap<float, float> groups = {
        {10, 1},
        {100, 10},
        {1000, 100}
    };

    rebuildSection(m_feedZSection, "FEEDRATE (Z)", options, groups);
}

void partMainJogParameters2::setStepSize(float value) {
    m_stepSection.currentValue = value;
    updateSectionUiState(m_stepSection);
}

void partMainJogParameters2::setFeedRateXY(float value) {
    m_feedXYSection.currentValue = value;
    updateSectionUiState(m_feedXYSection);
}

void partMainJogParameters2::setFeedRateZ(float value) {
    m_feedZSection.currentValue = value;
    updateSectionUiState(m_feedZSection);
}

void partMainJogParameters2::setSeparateZFeedrate(bool enabled) {
    if (m_feedZSection.mainFrame) {
        m_feedZSection.mainFrame->setVisible(enabled);
    }
}

bool partMainJogParameters2::isSeparateZFeedrate() const {
    return m_feedZSection.mainFrame ? m_feedZSection.mainFrame->isVisible() : false;
}

float partMainJogParameters2::getStepSize() const {
    return m_stepSection.currentValue;
}

float partMainJogParameters2::getFeedRateXY() const {
    return m_feedXYSection.currentValue;
}

float partMainJogParameters2::getFeedRateZ() const {
    return m_feedZSection.currentValue;
}

QMap<float, QList<float>> partMainJogParameters2::groupSelections(const QList<float>& selections, QMap<float, float>& groups)
{
    QMap<float, QList<float>> groupedSelections;
    for (float selection : selections) {
        float multiplier = 1.0f;
        for (auto it = groups.begin(); it != groups.end(); ++it) {
            if (selection < it.key()) {
                multiplier = it.value();
                break;
            }
        }
        groupedSelections[multiplier].append(selection / multiplier);
    }

    return groupedSelections;
}

void partMainJogParameters2::rebuildSection(Section& section, const QString& title, const QList<float>& options, QMap<float, float>& groups)
{
    if (section.mainFrame) {
        ui->mainRowsLayout->removeWidget(section.mainFrame);
        section.mainFrame->deleteLater();
        section.buttons.clear();
        section.valueLabel = nullptr;
    }

    section.currentOptions = options;

    QMap<float, QList<float>> groupedSelections = groupSelections(options, groups);

    section.mainFrame = new QFrame(this);
    section.mainFrame->setObjectName("sectionFrame");

    QGridLayout* gridLayout = new QGridLayout(section.mainFrame);
    gridLayout->setSpacing(1);
    gridLayout->setObjectName("gridLayout");
    gridLayout->setContentsMargins(1, 1, 1, 1);
    section.mainFrame->setLayout(gridLayout);

    // Header
    QFrame* header = createHeader(section.mainFrame, title, &section.valueLabel);
    gridLayout->addWidget(header, 0, 0, 1, options.size());

    int groupIndex = 1;
    int col = 0;
    for (auto it = groupedSelections.begin(); it != groupedSelections.end(); ++it) {
        float multiplier = it.key();
        QList<float> labels = it.value();

        // Group header
        QLabel *btnsGroupLabel = createGrpLabel(section.mainFrame, QString::number(multiplier), QString("_%1").arg(groupIndex));
        gridLayout->addWidget(btnsGroupLabel, 1, col, 1, labels.size());

        // Buttons
        for (float label : labels) {
            float realValue = label * multiplier;
            QPushButton *btn = createButton(section.mainFrame, QString::number(label), QString("_%1").arg(groupIndex), realValue, section);
            gridLayout->setColumnStretch(col, 1);
            gridLayout->addWidget(btn, 2, col++, 1, 1);
            section.buttons.append(btn);
        }

        groupIndex++;
    }

    ui->mainRowsLayout->addWidget(section.mainFrame);

    gridLayout->setRowStretch(0, 1); // header
    gridLayout->setRowStretch(1, 1); // groups
    gridLayout->setRowStretch(2, 2); // buttons
}

QFrame* partMainJogParameters2::createHeader(QWidget* parent, const QString& name, QLabel** outValueLabel)
{
    QFrame* headFrame = new QFrame(parent);
    headFrame->setObjectName("header");
    headFrame->setAutoFillBackground(false);
    headFrame->setFrameShape(QFrame::Shape::NoFrame);
    headFrame->setFrameShadow(QFrame::Shadow::Plain);

    QHBoxLayout* horizontalLayout = new QHBoxLayout(headFrame);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setObjectName("horizontalLayout");
    horizontalLayout->setContentsMargins(5, 1, 5, 1);

    QLabel* titleLabel = new QLabel(headFrame);
    titleLabel->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
    titleLabel->setText(name);
    horizontalLayout->addWidget(titleLabel);

    QLabel* valueLabel = new QLabel(headFrame);
    valueLabel->setObjectName("value");
    valueLabel->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);
    horizontalLayout->addWidget(valueLabel);

    if (outValueLabel) *outValueLabel = valueLabel;

    return headFrame;
}

QPushButton* partMainJogParameters2::createButton(QWidget* parent, const QString& text, const QString& tag, float realValue, Section& section)
{
    QPushButton *btn = new QPushButton(parent);
    btn->setObjectName("pushButton_5");
    btn->setFlat(true);
    btn->setProperty("tag", tag);
    btn->setText(text);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setCheckable(true);
    btn->setAutoExclusive(false);

    connect(btn, &QPushButton::clicked, this, [this, &section, realValue]() {
        section.currentValue = realValue;
        updateSectionUiState(section);

        if (&section == &m_stepSection) emit stepSizeChanged(realValue);
        else if (&section == &m_feedXYSection) emit feedRateXYChanged(realValue);
        else if (&section == &m_feedZSection) emit feedRateZChanged(realValue);
    });

    return btn;
}

QLabel *partMainJogParameters2::createGrpLabel(QWidget *parent, const QString& text, const QString& tag)
{
    QLabel *btnsGroupLabel = new QLabel(parent);
    btnsGroupLabel->setObjectName("label_4");
    btnsGroupLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
    btnsGroupLabel->setProperty("tag", tag);
    btnsGroupLabel->setText(text);

    return btnsGroupLabel;
}

void partMainJogParameters2::updateSectionUiState(Section& section)
{
    if (section.valueLabel) {
        section.valueLabel->setText(QString::number(section.currentValue));
    }
}
