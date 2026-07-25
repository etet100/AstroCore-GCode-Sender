#include "partmainjogparameters2.h"
#include "ui_partmainjogparameters2.h"
#include <QStyle>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QEvent>
#include <QVariant>
#include "utils/utils.h"

PartMainJogParameters2::PartMainJogParameters2(QWidget *parent) : AbstractPartMainJogParameters(parent), ui(new Ui::partMainJogParameters2)
{
    ui->setupUi(this);
    ui->sectionFrame->deleteLater();
    Utils::refreshStyle(this);
    Utils::setVisualMode(this, qApp->property("dark").toBool());

    m_stepSection.type = SectionType::Step;
    m_feedXYSection.type = SectionType::FeedXY;
    m_feedZSection.type = SectionType::FeedZ;

    m_updateTimer.setInterval(10);
    m_updateTimer.setSingleShot(true);
    connect(&m_updateTimer, &QTimer::timeout, this, [this]() {
        Utils::refreshStyle(m_stepSection.valueLabel);
        Utils::refreshStyle(m_feedXYSection.valueLabel);
        Utils::refreshStyle(m_feedZSection.valueLabel);
    });
}

void PartMainJogParameters2::setStepSizeOptions(const QStringList& options) {
    // max value, multiplier
    QMap<float, float> groups = {
        {1.0f, 0.1f},
        {10.0f, 1.0f},
        {100.0f, 10.0f},
        {1000.0f, 100.0f}
    };
    QList<float> floatOptions;
    for (const QString& opt : options) {
        floatOptions.append(opt.toFloat());
    }

    rebuildSection(m_stepSection, "STEP SIZE", floatOptions, groups);
}

void PartMainJogParameters2::setFeedRateXYOptions(const QStringList& options) {
    // max value, multiplier
    QMap<float, float> groups = {
        {10, 1},
        {100, 10},
        {1000, 100},
        {100000, 1000}
    };
    QList<float> floatOptions;
    for (const QString& opt : options) {
        floatOptions.append(opt.toFloat());
    }

    rebuildSection(m_feedXYSection, "FEEDRATE (XY)", floatOptions, groups);
}

void PartMainJogParameters2::setFeedRateZOptions(const QStringList& options) {
    // max value, multiplier
    QMap<float, float> groups = {
        {10, 1},
        {100, 10},
        {1000, 100},
        {100000, 1000}
    };
    QList<float> floatOptions;
    for (const QString& opt : options) {
        floatOptions.append(opt.toFloat());
    }

    rebuildSection(m_feedZSection, "FEEDRATE (Z)", floatOptions, groups);
}

void PartMainJogParameters2::setStepSize(float value) {
    m_stepSection.currentValue = value;
    updateSectionUiState(m_stepSection);
}

void PartMainJogParameters2::setFeedRateXY(float value) {
    m_feedXYSection.currentValue = value;
    updateSectionUiState(m_feedXYSection);
}

void PartMainJogParameters2::setFeedRateZ(float value) {
    m_feedZSection.currentValue = value;
    updateSectionUiState(m_feedZSection);
}

void PartMainJogParameters2::setSeparateZFeedrate(bool enabled) {
    if (m_feedZSection.mainFrame) {
        m_feedZSection.mainFrame->setVisible(enabled);
    }
}

float PartMainJogParameters2::stepSize() const {
    return m_stepSection.currentValue;
}

float PartMainJogParameters2::feedRateXY() const {
    return m_feedXYSection.currentValue;
}

float PartMainJogParameters2::feedRateZ() const {
    return m_feedZSection.currentValue;
}

QMap<float, QList<float>> PartMainJogParameters2::groupSelections(const QList<float>& selections, QMap<float, float>& groups)
{
    QMap<float, QList<float>> groupedSelections;
    for (float selection : selections) {
        float multiplier = -1.0f;
        for (auto it = groups.begin(); it != groups.end(); ++it) {
            if (selection < it.key()) {
                multiplier = it.value();
                break;
            }
        }
        // Values above the largest group bound (user can configure any step/feed)
        // fall into the last group instead of asserting.
        if (multiplier <= 0.0f) {
            multiplier = groups.isEmpty() ? 1.0f : std::prev(groups.end()).value();
        }

        groupedSelections[multiplier].append(selection / multiplier);
    }

    return groupedSelections;
}

void PartMainJogParameters2::rebuildSection(Section& section, const QString& title, const QList<float>& options, QMap<float, float>& groups)
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
        QLabel *btnsGroupLabel = createGrpLabel(section.mainFrame, "<small>x</small>" + QString::number(multiplier), QString("_%1").arg(groupIndex));
        gridLayout->addWidget(btnsGroupLabel, 1, col, 1, labels.size());

        // Buttons
        for (float label : labels) {
            float realValue = label * multiplier;
            StyledToolButton *btn = createButton(section.mainFrame, QString::number(label), QString("_%1").arg(groupIndex), realValue, section);
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

QFrame* PartMainJogParameters2::createHeader(QWidget* parent, const QString& name, QLabel** outValueLabel)
{
    QFrame* headFrame = new QFrame(parent);
    headFrame->setObjectName("header");
    headFrame->setAutoFillBackground(false);
    headFrame->setFrameShape(QFrame::Shape::NoFrame);
    headFrame->setFrameShadow(QFrame::Shadow::Plain);

    QHBoxLayout* horizontalLayout = new QHBoxLayout(headFrame);
    horizontalLayout->setSpacing(1);
    horizontalLayout->setContentsMargins(5, 1, 5, 1);

    QLabel* titleLabel = new QLabel(headFrame);
    titleLabel->setAlignment(Qt::AlignmentFlag::AlignLeading|Qt::AlignmentFlag::AlignLeft|Qt::AlignmentFlag::AlignVCenter);
    titleLabel->setText(name);
    horizontalLayout->addWidget(titleLabel);

    QLabel* valueLabel = new QLabel(headFrame);
    valueLabel->setAlignment(Qt::AlignmentFlag::AlignRight|Qt::AlignmentFlag::AlignTrailing|Qt::AlignmentFlag::AlignVCenter);
    horizontalLayout->addWidget(valueLabel);

    if (outValueLabel) { *outValueLabel = valueLabel; }

    return headFrame;
}

StyledToolButton* PartMainJogParameters2::createButton(QWidget* parent, const QString& text, const QString& tag, float realValue, Section& section)
{
    StyledToolButton *btn = new StyledToolButton(parent);

    btn->setProperty("tag", tag);
    btn->setText(text);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setAutoExclusive(false);
    btn->setProperty("section", section.type);
    btn->setProperty("value", realValue);
    btn->setMinimumHeight(28);
    btn->setSizePolicy(QSizePolicy(
        QSizePolicy::Policy::Preferred,
        QSizePolicy::Policy::Expanding
    ));
    btn->installEventFilter(this);

    connect(btn, &QPushButton::clicked, this, [this, &section, realValue]() {
        section.currentValue = realValue;
        updateSectionUiState(section);

        switch (section.type) {
            case SectionType::Step:
                emit stepSizeChanged(realValue);
                break;
            case SectionType::FeedXY:
                emit feedRateXYChanged(realValue);
                break;
            case SectionType::FeedZ:
                emit feedRateZChanged(realValue);
                break;
        }

        // mark value as final, not temporary
        section.valueLabel->setProperty("tag", "");
        Utils::refreshStyle(section.valueLabel);
    });

    return btn;
}

QLabel *PartMainJogParameters2::createGrpLabel(QWidget *parent, const QString& text, const QString& tag)
{
    QLabel *btnsGroupLabel = new QLabel(parent);

    btnsGroupLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
    btnsGroupLabel->setProperty("tag", tag);
    btnsGroupLabel->setText(text);

    return btnsGroupLabel;
}

void PartMainJogParameters2::updateSectionUiState(Section& section)
{
    if (section.valueLabel) {
        section.valueLabel->setText(QString::number(section.currentValue));
    }
}

bool PartMainJogParameters2::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() != QEvent::Enter && event->type() != QEvent::Leave) {
        return AbstractPartMainJogParameters::eventFilter(watched, event);
    }
    StyledToolButton *btn = qobject_cast<StyledToolButton*>(watched);
    if (!btn) {
        return AbstractPartMainJogParameters::eventFilter(watched, event);
    }

    SectionType sectionType = static_cast<SectionType>(btn->property("section").toInt());
    Section* section = nullptr;
    if (sectionType == SectionType::Step) section = &m_stepSection;
    else if (sectionType == SectionType::FeedXY) section = &m_feedXYSection;
    else if (sectionType == SectionType::FeedZ) section = &m_feedZSection;
    if (!section || !section->valueLabel) {
        return AbstractPartMainJogParameters::eventFilter(watched, event);
    }

    m_updateTimer.stop();
    float val = btn->property("value").toFloat();
    if (event->type() == QEvent::Enter) {
        if (qAbs(section->currentValue - val) > 0.01f) {
            // not the current value, show temporary
            section->valueLabel->setText(QString::number(val));
            section->valueLabel->setProperty("tag", "temp_value");
        } else {
            section->valueLabel->setText(QString::number(section->currentValue));
            section->valueLabel->setProperty("tag", "");
        }
    } else {
        section->valueLabel->setText(QString::number(section->currentValue));
        section->valueLabel->setProperty("tag", "");
    }
    m_updateTimer.start();

    return AbstractPartMainJogParameters::eventFilter(watched, event);
}

