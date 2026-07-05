#ifndef BOUNDINGBOXDRAWER_H
#define BOUNDINGBOXDRAWER_H

#include <QObject>
#include "shaderdrawable.h"
#include "billboarddrawable.h"
#include "core/gcode/parser/gcodeviewparser.h"

struct BoundingBoxBillboardContentData : public BillboardContentData
{
    BoundingBoxBillboardContentData(const QString& label, float x, float y, float z)
        : label(label), x(x), y(y), z(z) {}

    QString label;
    float x;
    float y;
    float z;
};

class BoundingBoxBillboardDrawer : public BillboardDrawable
{
    public:
        explicit BoundingBoxBillboardDrawer();

    protected:
        void drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data) override;
        QSize measureBillboard(const BillboardContentData* data) override;
        QString buildCacheKey(const BillboardContentData* data) override;
};

class BoundingBoxDrawer : public QObject, public ShaderDrawable
{
    Q_OBJECT

    public:
        explicit BoundingBoxDrawer();
        bool updateData(GLPalette &palette) override;
        void setViewParser(GCodeViewParser* viewParser);
        void setZoom(double zoom);
        void setVisible(bool visible) override;
        void toggleVisible();
        BillboardDrawable* billboardDrawable() { return &m_billboardDrawable; }

    private:
        double m_scale = 1.0;
        GCodeViewParser *m_viewParser = nullptr;
        BoundingBoxBillboardDrawer m_billboardDrawable;
        QVector3D minimumExtremes() override;
        QVector3D maximumExtremes() override;
};

#endif // BOUNDINGBOXDRAWER_H
