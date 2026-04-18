
#ifndef PARTMAINSTATE_H
#define PARTMAINSTATE_H

#include "abstractpartmainstate.h"

namespace Ui {
class partMainState;
}

class PartMainState : public AbstractPartMainState
{
    Q_OBJECT
public:
    explicit PartMainState(QWidget *parent);
    ~PartMainState();
    void setState(MachineState) override;
    void setWorkCoordinates(QVector3D) override;
    void setMachineCoordinates(QVector3D) override;
    void setUnits(Units units) override;
    void setStatusText(QString, QColor bgColor, QColor fgColor) override;

private:
    Ui::partMainState *ui;
    void up() override { setStatusText(QString(), "black", "white"); }
};

#endif // PARTMAINSTATE_H
