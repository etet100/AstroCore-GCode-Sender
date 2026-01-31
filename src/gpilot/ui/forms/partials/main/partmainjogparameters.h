#ifndef PARTMAINJOGPARAMETERS_H
#define PARTMAINJOGPARAMETERS_H

#include <QWidget>
#include "partmainjogparametersinterface.h"
#include "core/globals.h"
#include "core/config/module/configurationjogging.h"

namespace Ui {
class partMainJogParameters;
}

class PartMainJogParameters : public PartMainJogParametersInterface
{
        Q_OBJECT

    public:
        explicit PartMainJogParameters(QWidget* parent = nullptr);
        ~PartMainJogParameters();

        // int feedRate() const { return m_configurationJogging->feed(); };
        // int feedRateZ() const { return m_configurationJogging->finalFeedZ(); };
        // double stepSize() const { return m_configurationJogging->step(); };
        JoggingVector jogVector() const { return m_jogVector; };
        void configurationUpdated();

        void initialize(ConfigurationJogging &configurationJogging);

        // PartMainJogParametersInterface implementation
        void setStepSizeOptions(const QStringList& options) override;
        void setFeedRateXYOptions(const QStringList& options) override;
        void setFeedRateZOptions(const QStringList& options) override;

        void setStepSize(float value) override;
        void setFeedRateXY(float value) override;
        void setFeedRateZ(float value) override;

        void setSeparateZFeedrate(bool enabled) override;

        float stepSize() const override;
        float feedRateXY() const override;
        float feedRateZ() const override;

    private:
        Ui::partMainJogParameters *ui;
        bool m_initialized = false;
        ConfigurationJogging *m_configurationJogging;
        JoggingVector m_jogVector;
        void updateControls();
        void stopJogging();
        void stopJoggingIfContinuous();

    private slots:
        void onCmdFeedChanged(int index);
        void onCmdFeedZChanged(int index);
        void onCmdStepChanged(int index);
        void onChkSeparateZFeedToggled(bool);

    signals:
        void jog(JoggindDir dir, JoggingVector vector);
        void command(GRBLCommand command);
        void parametersChanged(int feed, double step, bool continuous);
        void stop();
};

#endif // PARTMAINJOGPARAMETERS_H
