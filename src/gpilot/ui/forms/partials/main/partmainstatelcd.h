
#ifndef PARTMAINSTATELCD_H
#define PARTMAINSTATELCD_H

#include "abstractpartmainstate.h"

namespace Ui {
class partMainStateLcd;
}

class PartMainStateLcd : public AbstractPartMainState
{
    Q_OBJECT
public:
    explicit PartMainStateLcd(QWidget *parent);
    ~PartMainStateLcd();
    void setState(MachineState) override;
    void setWorkCoordinates(QVector3D) override;
    void setMachineCoordinates(QVector3D) override;
    void setUnits(Units units) override;
    void setStatusText(QString status, QColor bgColor, QColor fgColor) override;
    void setConnectionName(QString name);
    void setConnectionState(bool connected);
    void setMachineStateReport(QString report);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Ui::partMainStateLcd *ui;
    QString formatPos(float val);
    void up() override { setStatusText(QString(), "black", "white"); }
};

#endif // PARTMAINSTATELCD_H
