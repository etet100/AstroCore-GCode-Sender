
#ifndef PARTMAINSTATELCD_H
#define PARTMAINSTATELCD_H

#include "partmainstatebase.h"

namespace Ui {
class partMainStateLcd;
}

class partMainStateLcd : public PartMainStateBase
{
    Q_OBJECT
public:
    explicit partMainStateLcd(QWidget *parent);
    void initialize(const Configuration &configuration) override;
    ~partMainStateLcd();
    void setState(MachineState) override;
    void setWorkCoordinates(QVector3D) override;
    void setMachineCoordinates(QVector3D) override;
    void setUnits(Units units) override;
    void setStatusText(QString, QString bgColor, QString fgColor) override;
    void setConName(QString name);

private:
    Ui::partMainStateLcd *ui;
    void initializeColorsAndCaptions() override;
    QString formatPos(float val);
    void up() override { setStatusText(QString(), "black", "white"); }
};

#endif // PARTMAINSTATELCD_H
