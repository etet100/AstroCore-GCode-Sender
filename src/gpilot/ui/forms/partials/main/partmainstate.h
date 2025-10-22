
#ifndef PARTMAINSTATE_H
#define PARTMAINSTATE_H

#include "partmainstatebase.h"

namespace Ui {
class partMainState;
}

class partMainState : public PartMainStateBase
{
    Q_OBJECT
public:
    explicit partMainState(QWidget *parent);
    ~partMainState();
    void setState(MachineState) override;
    void setWorkCoordinates(QVector3D) override;
    void setMachineCoordinates(QVector3D) override;
    void setUnits(Units units) override;
    void setStatusText(QString, QString bgColor, QString fgColor) override;

private:
    Ui::partMainState *ui;
    void up() override { setStatusText(QString(), "black", "white"); }
};

#endif // PARTMAINSTATE_H
