#ifndef BILLBOARDDRAWABLE_H
#define BILLBOARDDRAWABLE_H

#include "shaderdrawable.h"
#include <QOpenGLTexture>
#include <QImage>
#include <QMap>

struct BillboardData
{
    BillboardData() {}
    BillboardData(QVector3D pos, QString txt, QColor col = Qt::white, float size = 10.0f) {
        position = pos;
        text = txt;
        color = col;
        pixelSize = size;
    }

    QVector3D position;
    QString text;
    QColor color;
    float pixelSize;
};

struct BillboardVertex
{
    BillboardVertex() {}
    BillboardVertex(QVector3D pos, QVector2D size, QVector2D corner, QVector2D tex) {
        position = pos;
        billboardSize = size;
        cornerPos = corner;
        texCoord = tex;
    }

    QVector3D position;
    QVector2D billboardSize;
    QVector2D cornerPos;      // Which corner: (0,0), (1,0), (1,1), (0,1)
    QVector2D texCoord;
};

class BillboardDrawable : public ShaderDrawable
{
    public:
        explicit BillboardDrawable();
        ~BillboardDrawable();

        void addBillboard(const QVector3D &position, const QString &text, const QColor &color = Qt::white, float pixelSize = 10.0f);
        void clearBillboards();

        QOpenGLTexture* texture() { return m_texture; }

        void setScaleWithDistance(bool scale) { m_scaleWithDistance = scale; }
        bool scaleWithDistance() const { return m_scaleWithDistance; }
        void setGlobalScale(float scale) { m_globalScale = scale; }
        float globalScale() const { return m_globalScale; }

        ProgramType programType() override { return ProgramType::Billboard; }
        bool updateData(GLPalette &palette) override;
        void updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette) override;

        void draw(QOpenGLShaderProgram *shaderProgram) override;

        void init();

    private:
        QVector<BillboardData> m_billboards;
        QVector<BillboardVertex> m_billboardVertices;
        QOpenGLTexture *m_texture;
        QOpenGLBuffer m_indexBuffer;

        QMap<QString, QRectF> m_textCache;
        QImage m_atlasImage;
        int m_atlasX;
        int m_atlasY;
        int m_atlasRowHeight;

        bool m_scaleWithDistance;
        float m_globalScale;

        void rebuildAtlas(GLPalette &palette);
        QRectF addBillboardToAtlas(const QString &text, const QColor &textColor, const QFont &font);
        void addBillboardGeometry(const BillboardData &billboard, const QRectF &texRect, GLuint color);

        // Virtual method for customizing billboard appearance
        virtual void drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor);
};

#endif // BILLBOARDDRAWABLE_H
