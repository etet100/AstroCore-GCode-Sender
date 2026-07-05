#ifndef PARTMAINJOGPARAMETERS3_H
#define PARTMAINJOGPARAMETERS3_H

#include <QWidget>
#include <functional>
#include "abstractpartmainjogparameters.h"
#include <QToolButton>

namespace Ui {
class PartMainJogParameters3;
}

class PartMainJogParameters3 : public AbstractPartMainJogParameters
{
        Q_OBJECT

    public:
        explicit PartMainJogParameters3(QWidget* parent = nullptr);
        ~PartMainJogParameters3();

        void setStepSizeOptions(const QStringList& options) override;
        void setFeedRateXYOptions(const QStringList& options) override;
        void setFeedRateZOptions(const QStringList& options) override;

        void setStepSize(float value) override;
        void setFeedRateXY(float value) override;
        void setFeedRateZ(float value) override;

        void setSeparateZFeedrate(bool enabled) override;

        bool handlesContinuous() const override { return true; }
        void setContinuous() override;

        float stepSize() const override;
        float feedRateXY() const override;
        float feedRateZ() const override;

    private:
        void populateButtonGroup(QWidget* container, QList<QToolButton*>& buttons, const QStringList& options, bool infOption, std::function<void(float)> onSelected);
        void selectButton(QList<QToolButton*>& buttons, float value);

        Ui::PartMainJogParameters3* ui;
        QList<QToolButton*> m_stepButtons;
        QList<QToolButton*> m_feedXYButtons;
        QList<QToolButton*> m_feedZButtons;

        float m_stepSize = 0.0f;
        float m_feedRateXY = 0.0f;
        float m_feedRateZ = 0.0f;
};

#endif // PARTMAINJOGPARAMETERS3_H
