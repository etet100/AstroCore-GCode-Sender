#ifndef ORIGINBILLBOARDDRAWER_H
#define ORIGINBILLBOARDDRAWER_H

#include "billboarddrawable.h"

struct OriginBillboardContentData : public BillboardContentData
{
    OriginBillboardContentData(const QString& axis, const QColor& color) : axis(axis), color(color) {
    }

    QString axis;
    QColor color;
};

class OriginBillboardDrawer : public BillboardDrawable
{
    public:
        explicit OriginBillboardDrawer();

    protected:
        void drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data) override;
        QSize measureBillboard(const BillboardContentData *data) override;
        QString buildCacheKey(const BillboardContentData *data) override;
};

#endif // ORIGINBILLBOARDDRAWER_H
