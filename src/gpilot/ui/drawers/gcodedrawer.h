// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2025 BTS

#ifndef GCODEDRAWER_H
#define GCODEDRAWER_H

#include <QObject>
#include <QVector3D>
#include "core/gcode/parser/linesegment.h"
#include "core/gcode/parser/gcodeviewparser.h"
#include "shaderdrawable.h"
#include "ui/widgets/glpalette.h"

class GcodeDrawer : public QObject, public ShaderDrawable
{
    Q_OBJECT
public:
    enum GrayscaleCode { S, Z };

    explicit GcodeDrawer();

    void update();
    void update(QList<int> indexes);
    bool updateData(GLPalette &palette) override;

    QVector3D sizes() override;
    QVector3D minimumExtremes() override;
    QVector3D maximumExtremes() override;

    void setViewParser(GCodeViewParser* viewParser);
    GCodeViewParser* viewParser();

    // bool simplify() const;
    void setSimplify(bool simplify);

    // double simplifyPrecision() const;
    void setSimplifyPrecision(double simplifyPrecision);

    bool geometryUpdated();

    // QColor colorNormal() const;
    void setColorNormal(const QColor &colorNormal);
    // QColor colorHighlight() const;
    void setColorHighlight(const QColor &colorHighlight);
    // QColor colorZMovement() const;
    void setColorZMovement(const QColor &colorZMovement);
    // QColor colorRapidMovement() const;
    void setColorRapidMovement(const QColor &colorRapidMovement);
    // QColor colorDrawn() const;
    void setColorDrawn(const QColor &colorDrawn);
    // QColor colorStart() const;
    void setColorStart(const QColor &colorStart);
    // QColor colorEnd() const;
    void setColorEnd(const QColor &colorEnd);

    // bool getIgnoreZ() const;
    void setIgnoreZ(bool ignoreZ);

    // bool getGrayscaleSegments() const;
    void setGrayscaleSegments(bool grayscaleSegments);

    // GrayscaleCode grayscaleCode() const;
    void setGrayscaleCode(const GrayscaleCode &grayscaleCode);

    // int grayscaleMin() const;
    void setGrayscaleMin(int grayscaleMin);

    // int grayscaleMax() const;
    void setGrayscaleMax(int grayscaleMax);

    ProgramType programType() override { return ProgramType::GCode; };


public slots:
    void onLinesUpdated(int fromLine, int toLine);

private slots:
    void onTimerVertexUpdate();

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

    QTimer m_timerVertexUpdate;

    QImage m_image;
    QList<int> m_indexes;
    bool m_geometryUpdated;

    bool prepareVectors(GLPalette &palette);
    bool updateVectors(GLPalette &palette);

    QVector3D initialNormal(QVector3D p1, QVector3D p2);
    int getSegmentType(LineSegment& segment);
    GLuint getSegmentColor(LineSegment& segment, GLPalette &palette);
    GLuint getSegmentColorAndUpdateIndex(GLuint& var, GLuint index);
    void computeNormals();
    void generateBounds(GLPalette &palette);
};

#endif // GCODEDRAWER_H
