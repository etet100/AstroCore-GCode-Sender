#ifndef PARTMAINJOGPARAMETERS3_H
#define PARTMAINJOGPARAMETERS3_H

#include <QWidget>
#include <functional>
#include "partmainjogparametersinterface.h"
#include <QPushButton>

namespace Ui {
class PartMainJogParameters3;
}

class PartMainJogParameters3 : public PartMainJogParametersInterface
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

        float stepSize() const override;
        float feedRateXY() const override;
        float feedRateZ() const override;

    private:
        void populateButtonGroup(QWidget* container, QList<QPushButton*>& buttons, const QStringList& options, bool infOption, std::function<void(float)> onSelected);
        void selectButton(QList<QPushButton*>& buttons, float value);

        Ui::PartMainJogParameters3* ui;
        QList<QPushButton*> m_stepButtons;
        QList<QPushButton*> m_feedXYButtons;
        QList<QPushButton*> m_feedZButtons;

        float m_stepSize = 0.0f;
        float m_feedRateXY = 0.0f;
        float m_feedRateZ = 0.0f;
};

#endif // PARTMAINJOGPARAMETERS3_H
