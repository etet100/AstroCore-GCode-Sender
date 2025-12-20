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
        enum SectionType : int {
            Step = 0,
            FeedXY,
            FeedZ
        };
        inline const static QMap<SectionType, QString> sectionTypeName {
            {SectionType::Step, "step"},
            {SectionType::FeedXY, "feedXY"},
            {SectionType::FeedZ, "feedZ"}
        };

        Ui::partMainJogParameters2* ui;

        struct Section {
            QFrame* mainFrame = nullptr;
            QLabel* valueLabel = nullptr;
            QList<QPushButton*> buttons;
            float currentValue = 0.0f;
            QList<float> currentOptions;
            SectionType type;
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
