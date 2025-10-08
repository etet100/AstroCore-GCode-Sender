#ifndef PARTMAINOVERRIDE_H
#define PARTMAINOVERRIDE_H

#include "config/module/configurationmachine.h"
#include <QWidget>

namespace Ui {
class partMainOverride;
}

class partMainOverride : public QWidget
{
    Q_OBJECT

public:
    explicit partMainOverride(QWidget *parent = nullptr);
    ~partMainOverride();

    void applyConfiguration(ConfigurationMachine &machineConfiguration);
    // @TODO make this private
    Ui::partMainOverride *ui;

    static constexpr double NO_OVERRIDE = -1;
    class Overrides
    {
        public:
            bool feedOverridden;
            double feed;
            bool rapidOverridden;
            double rapid;
            bool spindleOverridden;
            double spindle;
    };
    Overrides overrides();

    void setRapid(int);
signals:
    void overrideChanged();

private slots:
    void onActSpindleSpeedMinusTriggered();
    void onActOverrideFeedPlusTriggered();
    void onActOverrideFeedMinusTriggered();
    void onActOverrideRapidPlusTriggered();
    void onActOverrideRapidMinusTriggered();
    void onActOverrideSpindlePlusTriggered();
    void onActOverrideSpindleMinusTriggered();
    void onOverrideChanged();
};

#endif // PARTMAINOVERRIDE_H
