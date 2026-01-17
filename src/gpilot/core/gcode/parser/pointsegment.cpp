// This file is a part of "Candle" application.
// This file was originally ported from "PointSegment.java" class
// of "Universal GcodeSender" application written by Will Winder
// (https://github.com/winder/Universal-G-Code-Sender)

// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include <QVector>

#include "pointsegment.h"

PointSegment::PointSegment()
{
}

PointSegment::PointSegment(PointSegment *ps)
{
    m_point = new QVector3D(ps->point()->x(), ps->point()->y(), ps->point()->z());
    m_lineNumber = ps->getLineNumber();
    m_toolhead = ps->getToolhead();
    m_speed = ps->getSpeed();
    m_isMetric = ps->isMetric();
    m_isZMovement = ps->isZMovement();
    m_isFastTraverse = ps->isFastTraverse();
    m_isAbsolute = ps->isAbsolute();

    if (ps->isArc()) {
        setArcCenter(ps->center());
        setRadius(ps->getRadius());
        setIsClockwise(ps->isClockwise());
        m_plane = ps->plane();
    }
}

PointSegment::PointSegment(const QVector3D *b, int num)
{
    m_point = new QVector3D(*b);
    m_lineNumber = num;
}

PointSegment::PointSegment(QVector3D *point, int num, QVector3D *center, double radius, bool clockwise)
{
    m_point = new QVector3D(*point);
    m_lineNumber = num;
    m_isArc = true;
    m_arcProperties = new ArcProperties();
    m_arcProperties->center = new QVector3D(center->x(), center->y(), center->z());
    m_arcProperties->radius = radius;
    m_arcProperties->isClockwise = clockwise;
}

PointSegment::~PointSegment()
{
    if (m_arcProperties != nullptr && m_arcProperties->center != nullptr) delete m_arcProperties->center;
    if (m_arcProperties != nullptr) delete m_arcProperties;
    if (m_point != nullptr) delete m_point;
}

QVector3D* PointSegment::point()
{
    return m_point;
}

QVector<double> PointSegment::points()
{
    QVector<double> points;
    points.append(m_point->x());
    points.append(m_point->y());

    return points;
}

void PointSegment::setToolHead(int head)
{
    m_toolhead = head;
}

int PointSegment::getToolhead()
{
    return m_toolhead;
}

int PointSegment::getLineNumber()
{
    return m_lineNumber;
}

void PointSegment::setSpeed(double s)
{
    m_speed = s;
}

double PointSegment::getSpeed()
{
    return m_speed;
}

void PointSegment::setIsZMovement(bool isZ)
{
    m_isZMovement = isZ;
}

bool PointSegment::isZMovement() {
    return m_isZMovement;
}

void PointSegment::setIsMetric(bool isMetric)
{
    m_isMetric = isMetric;
}

bool PointSegment::isMetric() {
    return m_isMetric;
}

void PointSegment::setIsArc(bool isA)
{
    m_isArc = isA;
}

bool PointSegment::isArc() {
    return m_isArc;
}

void PointSegment::setIsFastTraverse(bool isF)
{
    m_isFastTraverse = isF;
}

bool PointSegment::isFastTraverse() {
    return m_isFastTraverse;
}

// Arc properties.

void PointSegment::setArcCenter(QVector3D *center)
{
    if (m_arcProperties == nullptr) m_arcProperties = new ArcProperties();

    m_arcProperties->center = new QVector3D(center->x(), center->y(), center->z());
    setIsArc(true);
}

QVector<double> PointSegment::centerPoints()
{
    QVector<double> points;
    if (m_arcProperties != nullptr && m_arcProperties->center != nullptr) {
        points.append(m_arcProperties->center->x());
        points.append(m_arcProperties->center->y());
        points.append(m_arcProperties->center->z());
    }

    return points;
}

QVector3D *PointSegment::center()
{
    if (m_arcProperties != nullptr && m_arcProperties->center != nullptr) return m_arcProperties->center;

    return nullptr;
}

void PointSegment::setIsClockwise(bool clockwise)
{
    if (m_arcProperties == nullptr) m_arcProperties = new ArcProperties();
    m_arcProperties->isClockwise = clockwise;
}

bool PointSegment::isClockwise() {
    if (m_arcProperties != nullptr && m_arcProperties->center != nullptr) return m_arcProperties->isClockwise;

    return false;
}

void PointSegment::setRadius(double rad)
{
    if (m_arcProperties == nullptr) m_arcProperties = new ArcProperties();
    m_arcProperties->radius = rad;
}

double PointSegment::getRadius()
{
    if (m_arcProperties != nullptr && m_arcProperties->center != nullptr) return m_arcProperties->radius;

    return 0;
}

void PointSegment::convertToMetric()
{
    if (m_isMetric) {
        return;
    }

    m_isMetric = true;
    m_point->setX(m_point->x() * 25.4);
    m_point->setY(m_point->y() * 25.4);
    m_point->setZ(m_point->z() * 25.4);

    if (m_isArc && m_arcProperties != nullptr) {
        m_arcProperties->center->setX(m_arcProperties->center->x() * 25.4);
        m_arcProperties->center->setY(m_arcProperties->center->y() * 25.4);
        m_arcProperties->center->setZ(m_arcProperties->center->z() * 25.4);
        m_arcProperties->radius *= 25.4;
    }
}

bool PointSegment::isAbsolute() const
{
    return m_isAbsolute;
}

void PointSegment::setIsAbsolute(bool isAbsolute)
{
    m_isAbsolute = isAbsolute;
}

PointSegment::planes PointSegment::plane() const
{
    return m_plane;
}

void PointSegment::setPlane(const planes &plane)
{
    m_plane = plane;
}

double PointSegment::getSpindleSpeed() const
{
    return m_spindleSpeed;
}

void PointSegment::setSpindleSpeed(double spindleSpeed)
{
    m_spindleSpeed = spindleSpeed;
}

double PointSegment::getDwell() const
{
    return m_dwell;
}

void PointSegment::setDwell(double dwell)
{
    m_dwell = dwell;
}


