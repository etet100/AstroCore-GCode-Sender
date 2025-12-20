#ifndef PARTMAINJOGPARAMETERS_H
#define PARTMAINJOGPARAMETERS_H

#include <QWidget>
#include "partmainjogparametersinterface.h"
#include "core/globals.h"
#include "core/config/module/configurationjogging.h"

namespace Ui {
class partMainJogParameters;
}

class partMainJogParameters : public partMainJogParametersInterface
{
        Q_OBJECT

    public:
        explicit partMainJogParameters(QWidget* parent = nullptr);
        ~partMainJogParameters();

        int feedRate() const { return m_configurationJogging->feed(); };
        int feedRateZ() const { return m_configurationJogging->finalFeedZ(); };
        double stepSize() const { return m_configurationJogging->step(); };
        JoggingVector jogVector() const { return m_jogVector; };
        void configurationUpdated();

        void initialize(ConfigurationJogging &configurationJogging);

        // partMainJogParametersInterface implementation
        void setStepSizeOptions(const QList<float>& options) override;
        void setFeedRateXYOptions(const QList<float>& options) override;
        void setFeedRateZOptions(const QList<float>& options) override;

        void setStepSize(float value) override;
        void setFeedRateXY(float value) override;
        void setFeedRateZ(float value) override;

        void setSeparateZFeedrate(bool enabled) override;
        bool isSeparateZFeedrate() const override;

        float getStepSize() const override;
        float getFeedRateXY() const override;
        float getFeedRateZ() const override;

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
        void parametersChanged(int feed, double step);
        void stop();
};

#endif // PARTMAINJOGPARAMETERS_H
