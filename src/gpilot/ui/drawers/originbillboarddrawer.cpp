#include "originbillboarddrawer.h"
#include <QPainter>

OriginBillboardDrawer::OriginBillboardDrawer() : BillboardDrawable()
{
    m_depthTestEnabled = false;
}

void OriginBillboardDrawer::drawBillboard(QPainter& painter, const QRect& rect, const BillboardContentData* data_, const QString& text, const QColor& textColor)
{
    const OriginBillboardContentData* data = dynamic_cast<const OriginBillboardContentData*>(data_);
    assert(data != nullptr);

    QFont font;
    font.setPointSize(24);
    font.setBold(true);

    QFontMetrics fm(font);

    painter.setFont(font);
    painter.setPen(data->color);

    int xPos = rect.x() + (rect.width() - fm.horizontalAdvance(data->axis)) / 2;
    int yPos = rect.y() + (rect.height() - fm.height()) / 2 + fm.ascent();

    painter.drawText(xPos, yPos, data->axis);
}
