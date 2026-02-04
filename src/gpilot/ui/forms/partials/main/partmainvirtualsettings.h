// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINVIRTUALSETTINGS_H
#define PARTMAINVIRTUALSETTINGS_H

#include "core/globals.h"
#include "core/machine/physicalmachineconfiguration.h"
#include <QWidget>

namespace Ui {
class partMainVirtualSettings;
}

class PartMainVirtualSettings : public QWidget
{
    Q_OBJECT

    public:
        explicit PartMainVirtualSettings(QWidget *parent = nullptr);
        ~PartMainVirtualSettings();

        void deviceConfigurationReceived(PhysicalMachineConfiguration &machineConfiguration);

    signals:
        void lockProbeAtCurrentPosition();
        void resetProbePosition();
        void setHome(bool abs, double x, double y, double z);

    protected:
        void resizeEvent(QResizeEvent *event) override;

    private:
        Ui::partMainVirtualSettings *ui;
        HomingDirs m_homingDirs = HomingDirs();
        void set(int val);
        float calcFinalAxisPos(HomingDir homingDir, float currentPos, int val);
        void updateDevice();

    public slots:
        //void onConfigurationReceived(MachineConfiguration, QMap<int, double>);

    private slots:
        void onSetClicked1();
        void onSetClicked2();
        void onSetClicked3();
        void onEditingFinished();
        void onEStopToggled(bool value);
};

#endif // PARTMAINVIRTUALSETTINGS_H
