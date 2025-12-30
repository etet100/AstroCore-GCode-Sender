#include "originbillboarddrawer.h"
#include <QPainter>

OriginBillboardDrawer::OriginBillboardDrawer() : BillboardDrawable()
{
    m_depthTestEnabled = false;
}

void OriginBillboardDrawer::drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor)
{
    QFont font;
    font.setPointSize(24);
    font.setBold(true);

    QFontMetrics fm(font);

    painter.setFont(font);
    painter.setPen(textColor);

    int xPos = rect.x() + (rect.width() - fm.horizontalAdvance(text)) / 2;
    int yPos = rect.y() + (rect.height() - fm.height()) / 2 + fm.ascent();

    painter.drawText(xPos, yPos, text);
}
