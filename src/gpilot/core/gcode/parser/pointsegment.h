// This file is a part of "Candle" application.
// This file was originally ported from "PointSegment.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef POINTSEGMENT_H
#define POINTSEGMENT_H

#include <QVector3D>

#include "arcproperties.h"

class PointSegment
{
public:
    enum planes {
        XY,
        ZX,
        YZ
    };

    PointSegment();
    PointSegment(PointSegment *ps);
    PointSegment(const QVector3D *b, int num);
    PointSegment(QVector3D *point, int num, QVector3D *center, double radius, bool clockwise);
    ~PointSegment();
    QVector3D* point();

    QVector<double> points();
    int getToolhead();
    int getLineNumber();
    double getSpeed();
    void setSpeed(double s);
    bool isZMovement();
    void setIsZMovement(bool isZ);
    bool isMetric();
    void setIsMetric(bool m_isMetric);
    void setIsArc(bool isA);
    bool isArc();
    void setIsFastTraverse(bool isF);
    bool isFastTraverse();
    void setArcCenter(QVector3D *center);
    QVector<double> centerPoints();
    QVector3D *center();
    void setIsClockwise(bool clockwise);
    bool isClockwise();
    void setRadius(double rad);
    double getRadius();
    void convertToMetric();
    bool isAbsolute() const;
    void setIsAbsolute(bool isAbsolute);
    planes plane() const;
    void setPlane(const planes &plane);
    double getSpindleSpeed() const;
    void setSpindleSpeed(double spindleSpeed);
    double getDwell() const;
    void setDwell(double dwell);

private:
    ArcProperties *m_arcProperties = nullptr;
    int m_toolhead = 0;
    double m_speed = 0;
    double m_spindleSpeed = 0;
    double m_dwell = 0;
    QVector3D *m_point = nullptr;
    bool m_isMetric = true;
    bool m_isZMovement = false;
    bool m_isArc = false;
    bool m_isFastTraverse = false;
    bool m_isAbsolute = true;
    int m_lineNumber = -1;
    planes m_plane = XY;

    void setToolHead(int head);
};

#endif // POINTSEGMENT_H
