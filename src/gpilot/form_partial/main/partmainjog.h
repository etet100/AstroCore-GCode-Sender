#ifndef PARTMAINJOG_H
#define PARTMAINJOG_H

#include "config/module/configurationjogging.h"
#include "globals.h"
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

    constexpr static const double CONTINUOUS = -1;

    int feedRate() const { return m_feedRate; };
    double stepSize() const { return m_stepSize; };
    bool isContinuous() const { return m_stepSize == CONTINUOUS; };
    JoggingVector jogVector() const { return m_jogVector; };
    void storeAndResetKeyboardControl();
    bool keyboardControl();
    void setKeyboardControl(bool value);
    void configurationUpdated();
    void restoreKeyboardControl();

private:
    Ui::partMainJog *ui;
    int m_feedRate;
    double m_stepSize;
    bool m_storedKeyboardControl = false;
    ConfigurationJogging *m_configurationJogging;
    JoggingVector m_jogVector;
    void updateControls();
    void stopJogging();

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
    void onCmdFeedRateChanged(int index);
    void onCmdStepSizeChanged(int index);

signals:
    void jog(JoggindDir dir, JoggingVector vector);
    void command(GRBLCommand command);
    void parametersChanged(int feed, double step);
    void stop();
};

#endif // PARTMAINJOG_H
