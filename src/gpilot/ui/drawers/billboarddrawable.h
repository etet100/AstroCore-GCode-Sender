#ifndef BILLBOARDDRAWABLE_H
#define BILLBOARDDRAWABLE_H

#include "shaderdrawable.h"
#include <QOpenGLTexture>
#include <QImage>
#include <QMap>
#include <QSharedPointer>

struct BillboardContentData {
    virtual ~BillboardContentData() = default;
};

struct BillboardScreenPosition {
    QVector2D screenPos;     // Center position on screen
    QVector2D screenSize;    // Size in pixels on screen
    QSharedPointer<BillboardContentData> contentData;
    float zDepth;
};


struct BillboardData
{
    BillboardData() {}
    BillboardData(const QVector3D& pos, QSharedPointer<BillboardContentData> cdata, float size = 10.0f) {
        position = pos;
        pixelSize = size;
        contentData = cdata;
    }

    // Convenience constructor that accepts raw pointer and wraps it in QSharedPointer
    BillboardData(const QVector3D& pos, BillboardContentData* cdata, float size = 10.0f) {
        position = pos;
        pixelSize = size;
        contentData = QSharedPointer<BillboardContentData>(cdata);
    }

    QVector3D position;
    // QSharedPointer to allow polymorphic content data, this way we can extend BillboardContentData for different billboard types
    QSharedPointer<BillboardContentData> contentData;
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

        void addBillboard(const QVector3D& position, BillboardContentData* cdata, float pixelSize = 10.0f);
        void clearBillboards();

        void setScaleWithDistance(bool scale) { m_scaleWithDistance = scale; }
        void setGlobalScale(float scale) { m_globalScale = scale; }
        void setDebugBounds(bool enabled) { m_debugBounds = enabled; }

        ProgramType programType() override { return ProgramType::Billboard; }

        // For hit testing. Call updateScreenPositions() first to refresh positions.
        void updateScreenPositions(const QMatrix4x4& viewMatrix, const QMatrix4x4& projectionMatrix, const QSize& viewportSize, bool isOrthographic = false);
        // Returns content data of the billboard at the given screen position, or nullptr if none hit
        BillboardContentData* hitTest(const QPoint& screenPos) const;

        const QVector<BillboardScreenPosition>& screenPositions() const { return m_screenPositions; }
        bool updateData(GLPalette& palette) override;
        void updateGeometry(QOpenGLShaderProgram* shaderProgram, GLPalette& palette) override;
        void draw(QOpenGLShaderProgram* shaderProgram) override;
        void init();

    protected:
        bool m_scaleWithDistance = true;
        float m_globalScale;
        bool m_debugBounds = false;

        virtual QSize measureBillboard(const BillboardContentData* data) = 0;
        virtual QString buildCacheKey(const BillboardContentData* data) = 0;
        virtual void drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data) = 0;

    private:
        QVector<BillboardData> m_billboards;
        QVector<BillboardVertex> m_billboardVertices;
        QOpenGLTexture *m_texture;
        QOpenGLBuffer m_indexBuffer;
        QMap<QString, QRectF> m_textCache;
        QImage m_atlasImage;
        int m_atlasX;
        QVector<BillboardScreenPosition> m_screenPositions;
        int m_atlasY;
        int m_atlasRowHeight;

        void rebuildAtlas(GLPalette &palette);
        QRectF addBillboardToAtlas(const BillboardData& data);
        void addBillboardGeometry(const BillboardData& data, const QRectF& texRect);
};

#endif // BILLBOARDDRAWABLE_H
