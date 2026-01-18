// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "gcodedrawer.h"

GcodeDrawer::GcodeDrawer() : QObject()
{
    m_pointSize = 6;

    connect(&m_timerVertexUpdate, &QTimer::timeout, this, &GcodeDrawer::onTimerVertexUpdate);
    m_timerVertexUpdate.start(100);
}

void GcodeDrawer::update()
{
    m_indexes.clear();
    m_geometryUpdated = false;
    ShaderDrawable::update();
}

void GcodeDrawer::update(QList<int> indexes)
{
    // Store segments to update
    m_indexes += indexes;
}

bool GcodeDrawer::updateData(GLPalette &palette)
{
    if (m_indexes.isEmpty()) return prepareVectors(palette); else return updateVectors(palette);
}

QVector3D GcodeDrawer::initialNormal(QVector3D p1, QVector3D p2)
{
    QVector3D direction = p2 - p1;
    QVector3D normal;

    if (direction.z() == 0) {
        normal = QVector3D(0.0, -1.0, 0.1);
    } else {
        normal = QVector3D(-direction.z(), 0.0, direction.x());
    }

    assert(normal.length());

    return normal.normalized();
}

void GcodeDrawer::computeNormals()
{
    QVector3D normal;
    QVector3D lastNormal;
    QVector<VertexData> newPoints;
    for (int i = 0; i < m_lines.count() - 2; i += 2) {
        QVector3D tangent = m_lines[i + 1].position - m_lines[i].position;
        tangent.normalize();
        if (i == 0) {
            normal = QVector3D(0, 0, 1); // Dowolny wektor normalny dla pierwszego odcinka
            if (QVector3D::dotProduct(normal, tangent) != 0) {
                // Korekta normalnej, aby była ortogonalna do tangenta
                normal = QVector3D::crossProduct(tangent, QVector3D(0, 1, 0));
                normal.normalize();
            }
            m_lines[i].start = normal;
        } else {
            // Korekcja normalnej na podstawie poprzedniego kierunku
            QVector3D projectedNormal = QVector3D::crossProduct(tangent, lastNormal);
            normal = QVector3D::crossProduct(projectedNormal, tangent);
            normal.normalize();
            m_lines[i - 1].start = normal;
            m_lines[i].start = normal;
        }
        normal.normalize();
        lastNormal = normal;
    }
}

bool GcodeDrawer::prepareVectors(GLPalette &palette)
{
    assert(m_viewParser != nullptr);

    qDebug() << "[GcodeDrawer] Preparing vectors";

    QList<LineSegment> &list = m_viewParser->getLines();
    VertexData vertex;

    qDebug() << "[GcodeDrawer] Lines count" << list.count();

    // Clear all vertex data
    m_lines.clear();
    m_points.clear();
    m_triangles.clear();

    bool drawFirstPoint = true;
    float cumSegPosition = 0;
    for (int i = 0; i < list.count(); i++) {

        if (qIsNaN(list[i].getEnd().z())) {
            continue;
        }

        // Find first point of toolpath
        if (drawFirstPoint) {
            if (qIsNaN(list[i].getEnd().x()) || qIsNaN(list[i].getEnd().y())) continue;

            // Draw first toolpath point
            vertex.color = palette.color(m_colorStart);
            vertex.position = list[i].getEnd();
            if (m_ignoreZ) {
                vertex.position.setZ(0);
            }
            vertex.start = QVector3D(sNan, sNan, m_pointSize);
            m_points.append(vertex);

            drawFirstPoint = false;
            continue;
        }

        // Prepare vertices
        // if (list[i].isFastTraverse()) vertex.start = list[i].getStart();
        // else vertex.start = QVector3D(sNan, sNan, sNan);
        bool dashedLine = list[i].isZMovement() || list[i].isFastTraverse();

        // Simplify geometry
        int j = i;
        if (m_simplify && i < list.count() - 1) {
            QVector3D start = list[i].getEnd() - list[i].getStart();
            QVector3D next;
            double length = start.length();
            bool straight = false;

            do {
                list[i].setVertexIndex(m_lines.count()); // Store vertex index
                i++;
                if (i < list.count() - 1) {
                    next = list[i].getEnd() - list[i].getStart();
                    length += next.length();
//                    straight = start.crossProduct(start.normalized(), next.normalized()).length() < 0.025;
                }
            // Split short & straight lines
            } while ((length < m_simplifyPrecision || straight) && i < list.count()
                     && getSegmentType(list[i]) == getSegmentType(list[j]));
            i--;
        } else {
            list[i].setVertexIndex(m_lines.count()); // Store vertex index
        }

        vertex.color = getSegmentColor(list[i], palette);

        // ignore shorter than 0.0001
        float segmentLen = (list[i].getEnd() - list[j].getStart()).length();
        if (segmentLen > 0.0001) {
            // Line start
            vertex.position = list[j].getStart();
            vertex.cumSegPosition = dashedLine ? cumSegPosition : -1;
            if (m_ignoreZ) {
                vertex.position.setZ(0);
            }
            m_lines.append(vertex);

            cumSegPosition += segmentLen;

            // Line end
            vertex.position = list[i].getEnd();
            vertex.cumSegPosition = dashedLine ? cumSegPosition : -1;
            if (m_ignoreZ) {
                vertex.position.setZ(0);
            }
            m_lines.append(vertex);
        }

        // Draw last toolpath point
        if (i == list.count() - 1) {
            vertex.color = palette.color(m_colorEnd);
            vertex.position = list[i].getEnd();
            if (m_ignoreZ) {
                vertex.position.setZ(0);
            }
            vertex.start = QVector3D(sNan, sNan, m_pointSize);
            m_points.append(vertex);
        }
    }

    computeNormals();

    m_geometryUpdated = true;
    m_indexes.clear();

    return true;
}

bool GcodeDrawer::updateVectors(GLPalette &palette)
{
    // Update vertices
    QList<LineSegment>& list = m_viewParser->getLines();

    // Map buffer
    VertexData* data = (VertexData*)m_vbo.map(QOpenGLBuffer::WriteOnly);

    // Update vertices for each line segment
    int vertexIndex;
    foreach (int i, m_indexes) {
        // Update vertex pair
        if (i < 0 || i > list.count() - 1) continue;
        vertexIndex = list[i].vertexIndex();
        if (vertexIndex >= 0) {
            // Update vertex array
            if (data) {
                data[vertexIndex].color = getSegmentColor(list[i], palette);
                data[vertexIndex + 1].color = data[vertexIndex].color;
            } else {
                m_lines[vertexIndex].color = getSegmentColor(list[i], palette);
                m_lines[vertexIndex + 1].color = m_lines.at(vertexIndex).color;
            }
        }
    }

    m_indexes.clear();
    if (data) m_vbo.unmap();

    return !data;
}

GLuint GcodeDrawer::getSegmentColor(LineSegment& segment, GLPalette &palette)
{
    if (segment.drawn()) return m_colorDrawnIndex > -1 ? m_colorDrawnIndex : getSegmentColorAndUpdateIndex(m_colorDrawnIndex, palette.color(m_colorDrawn));
    else if (segment.isHightlight()) return m_colorHighlightIndex > -1 ? m_colorHighlightIndex : getSegmentColorAndUpdateIndex(m_colorHighlightIndex, palette.color(m_colorHighlight));
    else if (segment.isFastTraverse()) return m_colorRapidMovementIndex > -1 ? m_colorRapidMovementIndex : getSegmentColorAndUpdateIndex(m_colorRapidMovementIndex, palette.color(m_colorRapidMovement));
    else if (segment.isZMovement()) return m_colorZMovementIndex > -1 ? m_colorZMovementIndex : getSegmentColorAndUpdateIndex(m_colorZMovementIndex, palette.color(m_colorZMovement));
    else if (m_grayscaleSegments) {
        int grayscaleDiff = m_grayscaleMax - m_grayscaleMin;
        switch (m_grayscaleCode) {
        case GcodeDrawer::S:
            return palette.color(QColor::fromHsl(0, 0, QUANTIZE_COLOR(qBound<int>(0, 255 - 255.0 / grayscaleDiff * segment.getSpindleSpeed(), 255))));
        case GcodeDrawer::Z:
            return palette.color(QColor::fromHsl(0, 0, QUANTIZE_COLOR(qBound<int>(0, 255 - 255.0 / grayscaleDiff * segment.getStart().z(), 255))));
        }
    }

    return m_colorNormalIndex > -1 ? m_colorNormalIndex : getSegmentColorAndUpdateIndex(m_colorNormalIndex, palette.color(m_colorNormal));
}

int GcodeDrawer::getSegmentType(LineSegment& segment)
{
    return segment.isFastTraverse() + segment.isZMovement() * 2;
}

GLuint GcodeDrawer::getSegmentColorAndUpdateIndex(GLuint& var, GLuint index)
{
    var = index;

    return index;
}

QVector3D GcodeDrawer::sizes()
{
    QVector3D min = m_viewParser->getMinimumExtremes() - QVector3D(1, 1, 1);
    QVector3D max = m_viewParser->getMaximumExtremes() + QVector3D(1, 1, 1);

    return QVector3D(max.x() - min.x(), max.y() - min.y(), max.z() - min.z());
}

QVector3D GcodeDrawer::minimumExtremes()
{
    QVector3D v = m_viewParser->getMinimumExtremes();
    if (m_ignoreZ) {
        if (m_ignoreZ) v.setZ(0);
    }

    return v;
}

QVector3D GcodeDrawer::maximumExtremes()
{
    QVector3D v = m_viewParser->getMaximumExtremes();
    if (m_ignoreZ) {
        v.setZ(0);
    }

    return v;
}

void GcodeDrawer::setViewParser(GCodeViewParser* viewParser)
{
    // do not delete old parser!
    m_viewParser = viewParser;
}

GCodeViewParser *GcodeDrawer::viewParser()
{
    return m_viewParser;
}

void GcodeDrawer::setSimplify(bool simplify)
{
    m_simplify = simplify;
}

void GcodeDrawer::setSimplifyPrecision(double simplifyPrecision)
{
    m_simplifyPrecision = simplifyPrecision;
}

bool GcodeDrawer::geometryUpdated()
{
    return m_geometryUpdated;
}

void GcodeDrawer::setColorNormal(const QColor &colorNormal)
{
    m_colorNormal = colorNormal;
    m_colorNormalIndex = -1;
}

void GcodeDrawer::setColorHighlight(const QColor &colorHighlight)
{
    m_colorHighlight = colorHighlight;
}
\
void GcodeDrawer::setColorZMovement(const QColor &colorZMovement)
{
    m_colorZMovement = colorZMovement;
}

void GcodeDrawer::setColorRapidMovement(const QColor &colorRapidMovement)
{
    m_colorRapidMovement = colorRapidMovement;
}

void GcodeDrawer::setColorDrawn(const QColor &colorDrawn)
{
    m_colorDrawn = colorDrawn;
}

void GcodeDrawer::setColorStart(const QColor &colorStart)
{
    m_colorStart = colorStart;
}

void GcodeDrawer::setColorEnd(const QColor &colorEnd)
{
    m_colorEnd = colorEnd;
}

void GcodeDrawer::setIgnoreZ(bool ignoreZ)
{
    m_ignoreZ = ignoreZ;
}

void GcodeDrawer::onTimerVertexUpdate()
{
    if (!m_indexes.isEmpty()) ShaderDrawable::update();
}

void GcodeDrawer::setGrayscaleMax(int grayscaleMax)
{
    m_grayscaleMax = grayscaleMax;
}

void GcodeDrawer::onLinesUpdated(int fromLine, int toLine)
{

}

void GcodeDrawer::setGrayscaleMin(int grayscaleMin)
{
    m_grayscaleMin = grayscaleMin;
}

void GcodeDrawer::setGrayscaleCode(const GrayscaleCode &grayscaleCode)
{
    m_grayscaleCode = grayscaleCode;
}

void GcodeDrawer::setGrayscaleSegments(bool grayscaleSegments)
{
    m_grayscaleSegments = grayscaleSegments;
}
