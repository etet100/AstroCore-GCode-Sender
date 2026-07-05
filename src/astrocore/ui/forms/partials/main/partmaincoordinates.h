// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2026 BTS

#ifndef A53A685E_B4C0_4477_B407_51F56EA38849
#define A53A685E_B4C0_4477_B407_51F56EA38849

#ifndef PARTMAINCOORDINATES_H
#define PARTMAINCOORDINATES_H

#include <QWidget>
#include <QVector3D>

class MachineCoordinateCache;

namespace Ui {
class partMainCoordinates;
}

class PartMainCoordinates : public QWidget
{
    Q_OBJECT

public:
    explicit PartMainCoordinates(QWidget *parent = nullptr);
    ~PartMainCoordinates();

    void updateFromCache(const MachineCoordinateCache &cache);
    void setActiveRow(const QString &cs);

signals:
    void coordinateSystemSelected(const QString &cs);

private:
    Ui::partMainCoordinates *ui;
    QString m_activeCS;

    void setupTable();
};

#endif // PARTMAINCOORDINATES_H

#endif /* A53A685E_B4C0_4477_B407_51F56EA38849 */
