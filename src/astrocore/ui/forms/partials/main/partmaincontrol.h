// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef PARTMAINCONTROL_H
#define PARTMAINCONTROL_H

#include "core/globals.h"
#include "core/state_behavior/abstractstatebehavior.h"
#include "ui/utils/uistate.h"
#include <QWidget>
#include <QMenu>
#include <QAction>

namespace Ui {
class partMainControl;
}

class PartMainControl : public QWidget
{
    Q_OBJECT

public:
    explicit PartMainControl(QWidget *parent = nullptr);
    ~PartMainControl();
    void enable();
    void disable();
    void updateControlsState(const UiState& state);
    bool hold();
    void setFlood(bool);
    void initialize();

signals:
    void home();
    void reset();
    void unlock();
    void sleep();
    void door();
    void probe(ProbeMode mode);
    void zeroZ();
    void zeroXY();
    void check();
    void abortCheck();
    void scanTable();
    void command(GRBLCommand command);

private:
    Ui::partMainControl *ui;
    ProbeMode m_probeMode = ProbeMode::Single;
    QAction *m_actSingleProbe = nullptr;
    QAction *m_actDualProbe = nullptr;
    void setupProbeMenu();
    void updateProbeIcon();
    void textsVisible(bool visible);

private slots:
    void onCmdHomeClicked();
    void onCmdCheckClicked(bool checked);
    void onCmdResetClicked();
    void onCmdUnlockClicked();
    void onCmdHoldClicked(bool checked);
    void onCmdSleepClicked();
    void onCmdDoorClicked();
    void onCmdFloodClicked(bool checked);
    void onCmdProbeClicked();
    void onCmdZeroZClicked();
    void onCmdZeroXYClicked();
    void onCmdScanTableClicked();
};

#endif // PARTMAINCONTROL_H
