// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef GCODEDRAWER_H
#define GCODEDRAWER_H

#include <QObject>
#include <QVector3D>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>
#include "core/gcode/parser/linesegment.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include "shaderdrawable.h"
#include "ui/widgets/glpalette.h"

struct GcodeVectorData {
    QVector<VertexData> lines;
    QVector<VertexData> points;
    QVector<VertexData> triangles;
    bool success;
};

class GcodeDrawer : public QObject, public ShaderDrawable
{
    Q_OBJECT
public:
    enum GrayscaleCode { S, Z };

    explicit GcodeDrawer();
    ProgramType programType() override { return ProgramType::GCode; };

    void update();
    void update(QList<int> indexes);
    bool updateData(GLPalette &palette) override;

    QVector3D sizes() override;
    QVector3D minimumExtremes() override;
    QVector3D maximumExtremes() override;

    void setViewParser(GCodeViewParser* viewParser);
    GCodeViewParser* viewParser();
    void setSimplify(bool simplify);
    void setSimplifyPrecision(double simplifyPrecision);

    bool geometryUpdated();

    void setColorNormal(const QColor &colorNormal);
    void setColorHighlight(const QColor &colorHighlight);
    void setColorZMovement(const QColor &colorZMovement);
    void setColorRapidMovement(const QColor &colorRapidMovement);
    void setColorDrawn(const QColor &colorDrawn);
    void setColorStart(const QColor &colorStart);
    void setColorEnd(const QColor &colorEnd);
    void setIgnoreZ(bool ignoreZ);
    void setGrayscaleSegments(bool grayscaleSegments);
    void setGrayscaleCode(const GrayscaleCode &grayscaleCode);
    void setGrayscaleMin(int grayscaleMin);
    void setGrayscaleMax(int grayscaleMax);

public slots:
    void onLinesUpdated(int fromLine, int toLine);

private slots:
    void onTimerVertexUpdate();
    void onPrepareVectorsFinished();

private:
    GCodeViewParser *m_viewParser = nullptr;
    bool m_simplify;
    double m_simplifyPrecision;
    bool m_ignoreZ = false;
    bool m_grayscaleSegments = false;
    GrayscaleCode m_grayscaleCode = GcodeDrawer::S;
    int m_grayscaleMin = 0;
    int m_grayscaleMax = 255;

    QColor m_colorNormal;
    QColor m_colorDrawn;
    QColor m_colorHighlight;
    QColor m_colorZMovement;
    QColor m_colorStart;
    QColor m_colorEnd;
    QColor m_colorRapidMovement;

    GLuint m_colorNormalIndex = -1;
    GLuint m_colorDrawnIndex = -1;
    GLuint m_colorHighlightIndex = -1;
    GLuint m_colorZMovementIndex = -1;
    GLuint m_colorStartIndex = -1;
    GLuint m_colorEndIndex = -1;
    GLuint m_colorRapidMovementIndex = -1;
    GLuint m_colorGrayscaleIndex[QUANTIZE_COLOR_STEPS];
    void registerColorIndexes(GLPalette &palette);

    QTimer m_timerVertexUpdate;

    QImage m_image;
    QList<int> m_indexes;
    bool m_geometryUpdated;

    QFutureWatcher<GcodeVectorData> m_prepareVectorsWatcher;
    bool m_isPreparingVectors = false;

    bool prepareVectors(GLPalette &palette);
    GcodeVectorData prepareVectorsAsync();
    bool updateVectors(GLPalette &palette);
    GcodeVectorData updateVectorsAsync();

    QVector3D initialNormal(QVector3D p1, QVector3D p2);
    int getSegmentType(LineSegment& segment);
    GLuint getSegmentColor(LineSegment& segment, GLPalette &palette);
    GLuint getSegmentColor(LineSegment& segment);
    GLuint getSegmentColorAndUpdateIndex(GLuint& var, GLuint index);
    void computeNormals();
};

#endif // GCODEDRAWER_H
