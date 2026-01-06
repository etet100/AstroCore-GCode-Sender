#ifndef PARTMAINOVERRIDE_H
#define PARTMAINOVERRIDE_H

#include "core/config/module/configurationmachine.h"
#include <QWidget>

namespace Ui {
class partMainOverride;
}

class PartMainOverride : public QWidget
{
    Q_OBJECT

    public:
        explicit PartMainOverride(QWidget *parent = nullptr);
        ~PartMainOverride();

        void applyConfiguration(ConfigurationMachine &machineConfiguration);

        // Is it correct? No override means go 100%?
        static constexpr double NO_OVERRIDE = 100;

        void setCurrentRapid(int);
        void setCurrentFeed(int);
        void setCurrentSpindle(int);
        int targetFeed();
        int targetRapid();
        int targetSpindle();
        bool feedOverridden();
        bool rapidOverridden();
        bool spindleOverridden();

    signals:
        void overrideChanged(
            bool feedOverridden,
            double feed,
            bool rapidOverridden,
            double rapid,
            bool spindleOverridden,
            double spindle
        );

    private slots:
        void onFeedPlusTriggered();
        void onFeedMinusTriggered();
        void onRapidPlusTriggered();
        void onRapidMinusTriggered();
        void onSpindlePlusTriggered();
        void onSpindleMinusTriggered();
        void onOverridingToggled(bool state);
        void onValueChanged();

    private:
        Ui::partMainOverride *ui;

        void emitOverrideChanged();
};

#endif // PARTMAINOVERRIDE_H
