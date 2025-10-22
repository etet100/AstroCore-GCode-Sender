#ifndef PARTMAINJOG_H
#define PARTMAINJOG_H

#include "core/config/module/configurationjogging.h"
#include "core/globals.h"
#include <QWidget>

namespace Ui {
class partMainJog;
}

class partMainJog : public QWidget
{
    Q_OBJECT

public:
    explicit partMainJog(QWidget *parent = nullptr);
    void initialize(ConfigurationJogging &configurationJogging);
    ~partMainJog();

    int feedRate() const { return m_configurationJogging->feed(); };
    int feedRateZ() const { return m_configurationJogging->finalFeedZ(); };
    double stepSize() const { return m_configurationJogging->step(); };
    JoggingVector jogVector() const { return m_jogVector; };
    void storeAndResetKeyboardControl();
    bool keyboardControl();
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
    void onCmdZPlusPressed();
    void onCmdZPlusReleased();
    void onCmdZMinusPressed();
    void onCmdZMinusReleased();
    void onCmdStopClicked();
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

#endif // PARTMAINJOG_H
