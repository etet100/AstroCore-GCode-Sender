#ifndef PARTMAINJOG_H
#define PARTMAINJOG_H

#include "core/config/module/configurationjogging.h"
#include "core/globals.h"
#include <QWidget>

namespace Ui {
class partMainJog;
}

class PartMainJog : public QWidget
{
    Q_OBJECT

public:
    explicit PartMainJog(QWidget *parent = nullptr);
    void initialize(ConfigurationJogging &configurationJogging);
    ~PartMainJog();

    JoggingVector jogVector() const { return m_jogVector; };
    void storeAndResetKeyboardControl();
    void setKeyboardControl(bool value);
    void configurationUpdated();
    void restoreKeyboardControl();

private:
    Ui::partMainJog *ui;
    bool m_initialized = false;
    bool m_storedKeyboardControl = false;
    ConfigurationJogging *m_configurationJogging;
    JoggingVector m_jogVector;
    void updateControls();
    void stopJogging();
    void stopJoggingIfContinuous();

private slots:
    void onCmdYPlusPressed();
    void onCmdYPlusReleased();
    void onCmdYMinusPressed();
    void onCmdYMinusReleased();
    void onCmdXPlusPressed();
    void onCmdXPlusReleased();
    void onCmdXMinusPressed();
    void onCmdXMinusReleased();
    void onCmdXMinusYMinusPressed();
    void onCmdXMinusYMinusReleased();
    void onCmdXMinusYPlusPressed();
    void onCmdXMinusYPlusReleased();
    void onCmdXPlusYPlusPressed();
    void onCmdXPlusYPlusReleased();
    void onCmdXPlusYMinusPressed();
    void onCmdXPlusYMinusReleased();
    void onCmdZPlusPressed();
    void onCmdZPlusReleased();
    void onCmdZMinusPressed();
    void onCmdZMinusReleased();
    void onCmdStopClicked();
    void onChkSeparateZFeedToggled(bool);

signals:
    void jog(JoggindDir dir, JoggingVector vector);
    void command(GRBLCommand command);
    void parametersChanged(int feed, double step);
    void stop();
};

#endif // PARTMAINJOG_H

