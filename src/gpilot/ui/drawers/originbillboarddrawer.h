#ifndef ORIGINBILLBOARDDRAWER_H
#define ORIGINBILLBOARDDRAWER_H

#include "billboarddrawable.h"

class OriginBillboardDrawer : public BillboardDrawable
{
    public:
        explicit OriginBillboardDrawer();

    protected:
        void drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor) override;
};

#endif // ORIGINBILLBOARDDRAWER_H
