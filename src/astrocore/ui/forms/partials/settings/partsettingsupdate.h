#ifndef PARTSETTINGSUPDATE_H
#define PARTSETTINGSUPDATE_H

#include <QWidget>

namespace Ui {
class partSettingsUpdate;
}

class PartSettingsUpdate : public QWidget
{
        Q_OBJECT

    public:
        explicit PartSettingsUpdate(QWidget* parent = nullptr);
        ~PartSettingsUpdate();

        void setCheckForUpdates(bool value);
        bool checkForUpdates() const;

        void setCheckIntervalDays(int value);
        int checkIntervalDays() const;

    private:
        Ui::partSettingsUpdate* ui;
};

#endif // PARTSETTINGSUPDATE_H
