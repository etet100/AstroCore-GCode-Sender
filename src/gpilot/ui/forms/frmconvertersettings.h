// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#ifndef FRMCONVERTERSETTINGS_H
#define FRMCONVERTERSETTINGS_H

#include <QDialog>
#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QVariantMap>

class QFrame;
class QJsonDocument;

namespace Ui {
class FrmConverterSettings;
}

class FrmConverterSettings : public QDialog
{
        Q_OBJECT

    public:
        explicit FrmConverterSettings(QWidget* parent = nullptr);
        ~FrmConverterSettings();

        bool loadConfig(const QString& schema);
        bool loadConfigFromFile(const QString& path);

        QVariantMap values() const;
        // void setValues(const QVariantMap& values);

    private slots:
        void onRestoreDefaultsClicked();

    private:
        Ui::FrmConverterSettings* ui;
        QHash<QString, QWidget*> m_controls;
        QList<QFrame*> m_fieldBlocks;
        QJsonArray m_fieldsSchema;

        void populateHeader(const QJsonObject& root);
        void clearFields();
        void buildFields(const QJsonArray& fields);
        void applyDefaults();

        QFrame* createBoolField(const QJsonObject& field);
        QFrame* createIntField(const QJsonObject& field);
        QFrame* createFloatField(const QJsonObject& field);
        QFrame* createChoiceField(const QJsonObject& field);

        QFrame* createFieldBlock(const QJsonObject& field, QWidget* control);
        void registerControl(const QJsonObject& field, QWidget* control);
};

#endif // FRMCONVERTERSETTINGS_H
