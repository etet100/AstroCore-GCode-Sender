#ifndef PARTMAINJOGPARAMETERS2_H
#define PARTMAINJOGPARAMETERS2_H

#include "partmainjogparametersinterface.h"
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QMap>

namespace Ui {
class partMainJogParameters2;
}

class partMainJogParameters2 : public partMainJogParametersInterface
{
    Q_OBJECT

    public:
        explicit partMainJogParameters2(QWidget* parent = nullptr);

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
        Ui::partMainJogParameters2* ui;

        struct Section {
            QFrame* mainFrame = nullptr;
            QLabel* valueLabel = nullptr;
            QList<QPushButton*> buttons;
            float currentValue = 0.0f;
            QList<float> currentOptions;
        };

        Section m_stepSection;
        Section m_feedXYSection;
        Section m_feedZSection;

        // Helper methods
        void rebuildSection(Section& section, const QString& title, const QList<float>& options, QMap<float, float>& groups);
        QFrame* createHeader(QWidget* parent, const QString& name, QLabel** outValueLabel);
        QLabel* createGrpLabel(QWidget* parent, const QString& text, const QString& tag);
        QPushButton* createButton(QWidget* parent, const QString& text, const QString& tag, float realValue, Section& section);

        QMap<float, QList<float>> groupSelections(const QList<float>& selections, QMap<float, float>& groups);
        void updateSectionUiState(Section& section);

    protected:
        bool eventFilter(QObject *watched, QEvent *event) override;
};

#endif // PARTMAINJOGPARAMETERS2_H
