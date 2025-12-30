#ifndef ORIGINBILLBOARDDRAWABLE_H
#define ORIGINBILLBOARDDRAWABLE_H

#include "billboarddrawable.h"

class OriginBillboardDrawable : public BillboardDrawable
{
protected:
    void drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor) override;
};

#endif // ORIGINBILLBOARDDRAWABLE_H
