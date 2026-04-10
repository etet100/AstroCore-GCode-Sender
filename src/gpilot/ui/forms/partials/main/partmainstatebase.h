#ifndef PARTMAINSTATEBASE_H
#define PARTMAINSTATEBASE_H

#include "core/globals.h"
#include "core/config/configuration.h"
#include <QWidget>
#include <QVector3D>

class PartMainStateBase : public QWidget
{
    Q_OBJECT
public:
    explicit PartMainStateBase(QWidget *parent = nullptr) : QWidget(parent) {}
    virtual ~PartMainStateBase() {}

    virtual void initialize(const Configuration &configuration) {
        Q_UNUSED(configuration);
    };
    virtual void setState(MachineState state) = 0;
    virtual void setWorkCoordinates(QVector3D pos) = 0;
    virtual void setMachineCoordinates(QVector3D pos) = 0;
    virtual void setUnits(Units units) = 0;
    virtual void setStatusText(QString status, QColor bgColor, QColor fgColor) = 0;
    virtual void setBehaviorName(QString n) { m_behaviorName = n; up(); }
    virtual void setMachineState(QString n) { m_machineState = n; up(); }
    virtual void setMachineStateReport(QString report) = 0;

signals:
    void connectClicked();
    void disconnectClicked();

protected:
    QMap<MachineState, QString> m_statusCaptions;
    QMap<MachineState, QString> m_statusBackColors;
    QMap<MachineState, QString> m_statusForeColors;
    QString m_behaviorName;
    QString m_machineState;

    virtual void initializeColorsAndCaptions();
    virtual void up() { setStatusText(QString(), "black", "white"); }

signals:
    void grblCommand(GRBLCommand command);
};

#endif // PARTMAINSTATEBASE_H
