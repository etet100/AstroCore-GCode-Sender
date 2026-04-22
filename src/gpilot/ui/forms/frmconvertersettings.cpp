// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "frmconvertersettings.h"
#include "ui_frmconvertersettings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFrame>
#include <QJsonDocument>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

FrmConverterSettings::FrmConverterSettings(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::FrmConverterSettings)
{
    ui->setupUi(this);

    if (auto* btn = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
        connect(btn, &QPushButton::clicked, this, &FrmConverterSettings::onRestoreDefaultsClicked);
    }
}

FrmConverterSettings::~FrmConverterSettings()
{
    delete ui;
}

bool FrmConverterSettings::loadConfig(const QString& schema)
{
    QJsonDocument config = QJsonDocument::fromJson(schema.toUtf8());

    if (!config.isObject()) {
        return false;
    }

    const QJsonObject root = config.object();

    populateHeader(root);

    const QJsonValue fieldsValue = root.value("fields");
    m_fieldsSchema = fieldsValue.isArray() ? fieldsValue.toArray() : QJsonArray();
    buildFields(m_fieldsSchema);

    return true;
}

void FrmConverterSettings::populateHeader(const QJsonObject& root)
{
    const QString title = root.value("title").toString();
    const QString description = root.value("description").toString();
    const QString imagePath = root.value("image").toString();

    ui->titleLabel->setText(title);
    ui->titleLabel->setVisible(!title.isEmpty());

    ui->descriptionLabel->setText(description);
    ui->descriptionLabel->setVisible(!description.isEmpty());

    bool imageLoaded = false;
    if (!imagePath.isEmpty()) {
        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            const QSize maxSize(480, 240);
            ui->imageLabel->setPixmap(pixmap.scaled(
                maxSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
            imageLoaded = true;
        }
    }
    ui->imageLabel->setVisible(imageLoaded);

    const bool anyHeader = !title.isEmpty() || !description.isEmpty() || imageLoaded;
    ui->headerFrame->setVisible(anyHeader);
    ui->headerSeparator->setVisible(anyHeader);

    if (!title.isEmpty()) {
        setWindowTitle(title);
    }
}

bool FrmConverterSettings::loadConfigFromFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    return loadConfig(file.readAll());
}

QVariantMap FrmConverterSettings::values() const
{
    QVariantMap result;

    for (auto it = m_controls.constBegin(); it != m_controls.constEnd(); ++it) {
        QWidget* control = it.value();
        if (auto* cb = qobject_cast<QCheckBox*>(control)) {
            result.insert(it.key(), cb->isChecked());
        } else if (auto* sp = qobject_cast<QSpinBox*>(control)) {
            result.insert(it.key(), sp->value());
        } else if (auto* dsp = qobject_cast<QDoubleSpinBox*>(control)) {
            result.insert(it.key(), dsp->value());
        } else if (auto* combo = qobject_cast<QComboBox*>(control)) {
            result.insert(it.key(), combo->currentText());
        }
    }

    return result;
}

// void FrmConverterSettings::setValues(const QVariantMap& values)
// {
//     for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
//         QWidget* control = m_controls.value(it.key(), nullptr);
//         if (!control) {
//             continue;
//         }

//         if (auto* cb = qobject_cast<QCheckBox*>(control)) {
//             cb->setChecked(it.value().toBool());
//         } else if (auto* sp = qobject_cast<QSpinBox*>(control)) {
//             sp->setValue(it.value().toInt());
//         } else if (auto* dsp = qobject_cast<QDoubleSpinBox*>(control)) {
//             dsp->setValue(it.value().toDouble());
//         } else if (auto* combo = qobject_cast<QComboBox*>(control)) {
//             const int index = combo->findText(it.value().toString());
//             if (index >= 0) {
//                 combo->setCurrentIndex(index);
//             }
//         }
//     }
// }

void FrmConverterSettings::onRestoreDefaultsClicked()
{
    applyDefaults();
}

void FrmConverterSettings::clearFields()
{
    for (QFrame* block : std::as_const(m_fieldBlocks)) {
        block->deleteLater();
    }
    m_fieldBlocks.clear();
    m_controls.clear();
}

void FrmConverterSettings::buildFields(const QJsonArray& fields)
{
    clearFields();

    auto* layout = qobject_cast<QVBoxLayout*>(ui->scrollContent->layout());
    if (!layout) {
        return;
    }

    // Insert fields just before the trailing spacer.
    const int insertIndex = layout->count() - 1;

    for (const QJsonValue& value : fields) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject field = value.toObject();
        const QString type = field.value("type").toString();

        QFrame* block = nullptr;
        if (type == QLatin1String("bool")) {
            block = createBoolField(field);
        } else if (type == QLatin1String("int")) {
            block = createIntField(field);
        } else if (type == QLatin1String("float")) {
            block = createFloatField(field);
        } else if (type == QLatin1String("choice")) {
            block = createChoiceField(field);
        }

        if (block) {
            layout->insertWidget(insertIndex + m_fieldBlocks.size(), block);
            m_fieldBlocks.append(block);
        }
    }

    const bool hasFields = !m_fieldBlocks.isEmpty();
    ui->noFieldsLabel->setVisible(!hasFields);
    if (auto* btn = ui->buttonBox->button(QDialogButtonBox::RestoreDefaults)) {
        btn->setVisible(hasFields);
    }
}

void FrmConverterSettings::applyDefaults()
{
    for (const QJsonValue& value : std::as_const(m_fieldsSchema)) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject field = value.toObject();
        const QString name = field.value("name").toString();
        QWidget* control = m_controls.value(name, nullptr);
        if (!control) {
            continue;
        }

        const QJsonValue def = field.value("default");
        if (auto* cb = qobject_cast<QCheckBox*>(control)) {
            cb->setChecked(def.toBool());
        } else if (auto* sp = qobject_cast<QSpinBox*>(control)) {
            sp->setValue(def.toInt());
        } else if (auto* dsp = qobject_cast<QDoubleSpinBox*>(control)) {
            dsp->setValue(def.toDouble());
        } else if (auto* combo = qobject_cast<QComboBox*>(control)) {
            const int index = combo->findText(def.toString());
            if (index >= 0) {
                combo->setCurrentIndex(index);
            }
        }
    }
}

QFrame* FrmConverterSettings::createBoolField(const QJsonObject& field)
{
    auto* checkBox = new QCheckBox();
    checkBox->setChecked(field.value("default").toBool(false));

    registerControl(field, checkBox);

    return createFieldBlock(field, checkBox);
}

QFrame* FrmConverterSettings::createIntField(const QJsonObject& field)
{
    auto* spin = new QSpinBox();
    spin->setMinimum(field.value("min").toInt(std::numeric_limits<int>::min()));
    spin->setMaximum(field.value("max").toInt(std::numeric_limits<int>::max()));
    spin->setValue(field.value("default").toInt(0));

    registerControl(field, spin);

    return createFieldBlock(field, spin);
}

QFrame* FrmConverterSettings::createFloatField(const QJsonObject& field)
{
    auto* spin = new QDoubleSpinBox();
    spin->setDecimals(field.value("decimals").toInt(3));
    spin->setMinimum(field.value("min").toDouble(-std::numeric_limits<double>::max()));
    spin->setMaximum(field.value("max").toDouble(std::numeric_limits<double>::max()));
    spin->setSingleStep(field.value("step").toDouble(0.1));
    spin->setValue(field.value("default").toDouble(0.0));

    registerControl(field, spin);

    return createFieldBlock(field, spin);
}

QFrame* FrmConverterSettings::createChoiceField(const QJsonObject& field)
{
    auto* combo = new QComboBox();
    const QJsonArray options = field.value("options").toArray();
    for (const QJsonValue& opt : options) {
        combo->addItem(opt.toString());
    }

    const int defaultIndex = combo->findText(field.value("default").toString());
    if (defaultIndex >= 0) {
        combo->setCurrentIndex(defaultIndex);
    }

    registerControl(field, combo);

    return createFieldBlock(field, combo);
}

QFrame* FrmConverterSettings::createFieldBlock(const QJsonObject& field, QWidget* control)
{
    auto* frame = new QFrame();
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setFrameShadow(QFrame::Raised);

    auto* layout = new QVBoxLayout(frame);
    layout->setContentsMargins(8, 6, 8, 8);
    layout->setSpacing(2);

    auto* label = new QLabel(field.value("label").toString(field.value("name").toString()));
    QFont labelFont = label->font();
    labelFont.setBold(true);
    label->setFont(labelFont);
    layout->addWidget(label);

    const QString description = field.value("description").toString();
    if (!description.isEmpty()) {
        auto* desc = new QLabel(description);
        desc->setWordWrap(true);
        desc->setStyleSheet(QStringLiteral("color: gray;"));
        layout->addWidget(desc);
    }

    layout->addWidget(control);

    return frame;
}

void FrmConverterSettings::registerControl(const QJsonObject& field, QWidget* control)
{
    const QString name = field.value("name").toString();
    if (name.isEmpty()) {
        return;
    }

    control->setObjectName(name);
    m_controls.insert(name, control);
}
