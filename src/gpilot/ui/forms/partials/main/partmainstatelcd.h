#ifndef PARTMAINSTATELCD_H
#define PARTMAINSTATELCD_H

#include "core/globals.h"
#include "core/config/configuration.h"
#include <QWidget>
#include <QVector3D>

namespace Ui {
class partMainStateLcd;
}

class partMainStateLcd : public QWidget
{
    Q_OBJECT

    public:
        explicit partMainStateLcd(QWidget *parent);
        void initialize(const Configuration &configuration);
        ~partMainStateLcd();
        void setState(DeviceState);
        void setWorkCoordinates(QVector3D);
        void setMachineCoordinates(QVector3D);
        void setUnits(Units units);
        void setStatusText(QString, QString bgColor, QString fgColor);
        void setConName(QString name);

    private:
        Ui::partMainStateLcd *ui;
      //  const Configuration &m_configuration;
        QMap<DeviceState, QString> m_statusCaptions;
        QMap<DeviceState, QString> m_statusBackColors;
        QMap<DeviceState, QString> m_statusForeColors;

        void initializeColorsAndCaptions();
        QString formatPos(float val);

    signals:
        void grblCommand(GRBLCommand command);

};

#endif // PARTMAINSTATELCD_H
