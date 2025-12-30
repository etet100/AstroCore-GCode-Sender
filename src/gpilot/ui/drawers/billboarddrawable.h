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
    BillboardVertex(QVector3D pos, QVector2D size, QVector2D corner, QVector2D tex, GLuint col) {
        position = pos;
        billboardSize = size;
        cornerPos = corner;
        texCoord = tex;
        color = col;
    }

    QVector3D position;
    QVector2D billboardSize;
    QVector2D cornerPos;      // Which corner: (0,0), (1,0), (1,1), (0,1)
    QVector2D texCoord;
    GLuint color;
};

class BillboardDrawable : public ShaderDrawable
{
    public:
        explicit BillboardDrawable();
        ~BillboardDrawable();

        void addBillboard(const QVector3D &position, const QString &text, const QColor &color = Qt::white, float pixelSize = 10.0f);
        void clearBillboards();

        QOpenGLTexture* texture() { return m_texture; }

        // Billboard rendering options
        void setScaleWithDistance(bool scale) { m_scaleWithDistance = scale; }
        bool scaleWithDistance() const { return m_scaleWithDistance; }
        void setGlobalScale(float scale) { m_globalScale = scale; }
        float globalScale() const { return m_globalScale; }

        ProgramType programType() override { return ProgramType::Billboard; }
        bool updateData(GLPalette &palette) override;
        void updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette) override;

        void draw(QOpenGLShaderProgram *shaderProgram) override;

        void init();  // Not virtual in base class, so no override

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

        // Rendering options
        bool m_scaleWithDistance;  // true = scale with distance (perspective), false = constant screen size
        float m_globalScale;       // Global scale multiplier (default 1.0)

        void rebuildAtlas(GLPalette &palette);
        QRectF addTextToAtlas(const QString &text, const QColor &textColor, const QFont &font);
        void addBillboardGeometry(const BillboardData &billboard, const QRectF &texRect, GLuint color);

        // Virtual method for customizing billboard appearance
        virtual void drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor);
};

#endif // BILLBOARDDRAWABLE_H
