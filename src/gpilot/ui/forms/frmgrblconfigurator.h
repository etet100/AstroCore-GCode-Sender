// This file is a part of "G-Pilot (formerly Candle)" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef FRMGRBLCONFIGURATOR_H
#define FRMGRBLCONFIGURATOR_H

#include <QDialog>
#include "communicator.h"
#include <CPropertyHeader.h>
#include <CBaseProperty.h>

namespace Ui {
class frmGrblConfigurator;
}

enum Type {
    Boolean,
    Integer,
    Double,
    Axes,
};

enum class Axis : int {
    None = -1,
    X = 0,
    Y = 1,
    Z = 2,
};

struct ConfigEntry {
    int index;
    QString description;
    QString meaning;
    Type type;
    CBaseProperty *property = nullptr;
    QMap<Axis, CBaseProperty*> properties = {};
};

class frmGrblConfigurator : public QDialog
{
    Q_OBJECT

    public:
        explicit frmGrblConfigurator(QWidget *parent, ConfigurationUI &uiConfiguration, Communicator *communicator);
        ~frmGrblConfigurator();

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
        bool m_isSaving = false;
        void setInfo(QString text, QColor color);
        bool m_firstShow = true;
        QMap<Axis, CBaseProperty*> addAxesProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addBooleanProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addIntegerProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addDoubleProperty(CPropertyHeader *, ConfigEntry);
        CBaseProperty* addAxisProperty(CPropertyHeader *header, ConfigEntry entry, Axis axis);
        void setSettingsBit(int, int, bool);
        void accept() override;
        void findParametersToBeSaved(QMap<int, double>);

    private slots:
        void onConfigurationReceived(MachineConfiguration, QMap<int, double>);
        void update();
        void onUpdateClicked();
        void itemChanged(QTreeWidgetItem *item, int column);
};

#endif // FRMGRBLCONFIGURATOR_H
