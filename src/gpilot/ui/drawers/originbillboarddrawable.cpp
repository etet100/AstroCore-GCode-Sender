#include "originbillboarddrawable.h"
#include <QPainter>

void OriginBillboardDrawable::drawBillboard(QPainter &painter, const QRect &rect, const QString &text, const QColor &textColor)
{
    // No background - just transparent

    // Use large, bold font for axis labels
    QFont font;
    font.setPointSize(32);
    font.setBold(true);

    QFontMetrics fm(font);

    // Draw text centered with the billboard's color
    painter.setFont(font);
    painter.setPen(textColor);

    int xPos = rect.x() + (rect.width() - fm.horizontalAdvance(text)) / 2;
    int yPos = rect.y() + (rect.height() - fm.height()) / 2 + fm.ascent();

    painter.drawText(xPos, yPos, text);
}
