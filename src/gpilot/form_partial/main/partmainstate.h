#ifndef PARTMAINSTATE_H
#define PARTMAINSTATE_H

#include "globals.h"
#include "../../config/configuration.h"
#include <QWidget>
#include <QVector3D>

namespace Ui {
class partMainState;
}

class partMainState : public QWidget
{
    Q_OBJECT

    public:
        explicit partMainState(QWidget *parent);
        void initialize(const Configuration &configuration);
        ~partMainState();
        void setState(DeviceState);
        void setWorkCoordinates(QVector3D);
        void setMachineCoordinates(QVector3D);
        void setUnits(Units units);
        void setStatusText(QString, QString bgColor, QString fgColor);

    private:
        Ui::partMainState *ui;
      //  const Configuration &m_configuration;
        QMap<DeviceState, QString> m_statusCaptions;
        QMap<DeviceState, QString> m_statusBackColors;
        QMap<DeviceState, QString> m_statusForeColors;

        void initializeColorsAndCaptions();

    signals:
        void grblCommand(GRBLCommand command);

};

#endif // PARTMAINSTATE_H
