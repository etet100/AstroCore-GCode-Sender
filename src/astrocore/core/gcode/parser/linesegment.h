// This file is a part of "Candle" application.
// This file was originally ported from "LineSegment.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef LINESEGMENT_H
#define LINESEGMENT_H

#include <QVector3D>
#include "pointsegment.h"

class LineSegment
{
public:
    LineSegment();
    LineSegment(QVector3D a, QVector3D b, int num);
    LineSegment(LineSegment *initial);
    ~LineSegment();

    bool operator==(const LineSegment &other) const {
        return m_lineNumber == other.m_lineNumber &&
                m_first == other.m_first &&
               m_second == other.m_second;
    }

    int getLineNumber() const;
    QList<QVector3D> getPointArray();
    QList<double> getPoints();

    QVector3D &getStart();
    const QVector3D &getStart() const;
    void setStart(QVector3D vector);

    QVector3D &getEnd();
    const QVector3D &getEnd() const;
    void setEnd(QVector3D vector);

    void setToolHead(int head);
    int getToolhead() const;
    void setSpeed(double s);
    double getSpeed() const;
    void setIsZMovement(bool isZ);
    bool isZMovement() const;
    void setIsArc(bool isA);
    bool isArc() const;
    void setIsFastTraverse(bool isF);
    bool isFastTraverse() const;

    bool contains(const QVector3D &point);

    bool drawn() const;
    void setDrawn(bool drawn);

    bool isMetric() const;
    void setIsMetric(bool isMetric);

    bool isAbsolute() const;
    void setIsAbsolute(bool isAbsolute);

    bool isHightlight() const;
    void setIsHightlight(bool isHightlight);

    int vertexIndex() const;
    void setVertexIndex(int vertexIndex);

    double getSpindleSpeed() const;
    void setSpindleSpeed(double spindleSpeed);

    double getDwell() const;
    void setDwell(double dwell);

    bool isClockwise() const;
    void setIsClockwise(bool isClockwise);

    PointSegment::planes plane() const;
    void setPlane(const PointSegment::planes &plane);

private:
    int m_toolhead = 0;
    double m_speed = 0;
    double m_spindleSpeed = 0;
    double m_dwell = 0;
    QVector3D m_first, m_second;

    // Line properties
    bool m_isZMovement = false;
    bool m_isArc = false;
    bool m_isClockwise = false;
    bool m_isFastTraverse = false;
    int m_lineNumber = -1;
    bool m_drawn = false;
    bool m_isMetric = true;
    bool m_isAbsolute = true;
    bool m_isHightlight = false;
    int m_vertexIndex = -1;

    PointSegment::planes m_plane = PointSegment::XY;
};

#endif // LINESEGMENT_H
