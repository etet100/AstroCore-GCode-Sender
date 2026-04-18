// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef FRMGRBLCONFIGURATOR_H
#define FRMGRBLCONFIGURATOR_H

#include <QDialog>
#include "core/communicator/communicator.h"
#include <CPropertyHeader.h>
#include <CBaseProperty.h>
#include <QCoroTask>
#include <optional>

namespace Ui {
class frmGrblConfigurator;
}

enum Type {
    Boolean,
    Integer,
    Double,
    Axes,
};

struct ConfigEntry {
    int index;
    QString description;
    QString meaning;
    Type type;
    CBaseProperty *property = nullptr;
    QMap<Axis, CBaseProperty*> properties = {};
};

class FrmGrblConfigurator : public QDialog
{
    Q_OBJECT

    public:
        explicit FrmGrblConfigurator(QWidget *parent, ConfigurationUI &uiConfiguration, Communicator *communicator);
        ~FrmGrblConfigurator();

    protected:
        void showEvent(QShowEvent *se) override;
        void resizeEvent(QResizeEvent *re) override;
        void changeEvent(QEvent *ce) override;
        void moveEvent(QMoveEvent *me) override;

    private:
        Ui::frmGrblConfigurator *ui;
        ConfigurationUI &m_uiConfiguration;
        Communicator *m_communicator;
        QMap<int, double> m_currentSettings;
        std::optional<QCoro::Task<void>> m_activeTask;
        void setInfo(QString text, QColor color = Qt::transparent);
        bool m_firstShow = true;
        QMap<Axis, CBaseProperty*> addAxesProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addBooleanProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addIntegerProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addDoubleProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addAxisProperty(CPropertyHeader *header, ConfigEntry entry, Axis axis);
        void setSettingsBit(int, int, bool);
        void accept() override;
        void findParametersToBeSaved(QMap<int, double>);
        QCoro::Task<void> update();
        QCoro::Task<void> save();

    private slots:
        void onUpdateClicked();
        void itemChanged(QTreeWidgetItem *item, int column);
};

#endif // FRMGRBLCONFIGURATOR_H
