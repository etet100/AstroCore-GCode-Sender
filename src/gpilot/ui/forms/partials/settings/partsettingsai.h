#ifndef PARTSETTINGSAI_H
#define PARTSETTINGSAI_H

#include <QWidget>

namespace Ui {
class partSettingsAI;
}

class PartSettingsAI : public QWidget
{
        Q_OBJECT

    public:
        explicit PartSettingsAI(QWidget* parent = nullptr);
        ~PartSettingsAI();

        void setOpenAIKey(const QString& key);
        QString openAIKey() const;

    private:
        Ui::partSettingsAI* ui;
};

#endif // PARTSETTINGSAI_H
