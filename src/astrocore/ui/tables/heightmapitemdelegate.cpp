// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2025 BTS

#include "heightmapitemdelegate.h"
#include "heightmaptablemodel.h"
#include "ui/utils/thememanager.h"
#include <QDoubleSpinBox>
#include <QPainter>
#include <QApplication>

HeightmapItemDelegate::HeightmapItemDelegate(double min, double max, QObject* parent)
    : QStyledItemDelegate(parent)
    , m_min(min)
    , m_max(max)
{
}

QWidget* HeightmapItemDelegate::createEditor(QWidget* parent,
                                              const QStyleOptionViewItem&,
                                              const QModelIndex&) const
{
    auto* spin = new QDoubleSpinBox(parent);
    spin->setRange(m_min, m_max);
    spin->setDecimals(3);
    spin->setSingleStep(0.001);
    return spin;
}

void HeightmapItemDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* spin = qobject_cast<QDoubleSpinBox*>(editor);
    if (!spin) return;
    spin->setValue(index.data(Qt::EditRole).toDouble());
}

void HeightmapItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                          const QModelIndex& index) const
{
    auto* spin = qobject_cast<QDoubleSpinBox*>(editor);
    if (!spin) return;
    spin->interpretText();
    model->setData(index, spin->value(), Qt::EditRole);
}

void HeightmapItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    opt.text.clear();

    QStyle* style = opt.widget ? opt.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    QString valText = index.data(Qt::DisplayRole).toString();
    QVariant posVar = index.data(HeightmapTableModel::PositionMmRole);
    QString posText;
    if (posVar.isValid()) {
        QPointF pos = posVar.toPointF();
        posText = QString("%1, %2").arg(pos.x(), 0, 'f', 1).arg(pos.y(), 0, 'f', 1);
    }

    const QRect r = option.rect.adjusted(2, 2, -2, -2);
    const int topHeight = r.height() * 4 / 10;
    const QRect topRect(r.x(), r.y(), r.width(), topHeight);
    const QRect bottomRect(r.x(), r.y() + topHeight, r.width(), r.height() - topHeight);

    painter->save();

    QFont smallFont = option.font;
    smallFont.setPointSizeF(qMax(6.0, smallFont.pointSizeF() * 0.78));
    painter->setFont(smallFont);
    QColor grey = option.palette.color(QPalette::Disabled, QPalette::WindowText);
    painter->setPen(grey);
    painter->drawText(topRect, Qt::AlignCenter, posText);

    painter->setFont(option.font);
    QPalette::ColorRole textRole = (option.state & QStyle::State_Selected)
        ? QPalette::HighlightedText
        : QPalette::Text;
    painter->setPen(option.palette.color(textRole));
    painter->drawText(bottomRect, Qt::AlignCenter, valText);

    painter->restore();
}

QSize HeightmapItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                       const QModelIndex&) const
{
    QFont smallFont = option.font;
    smallFont.setPointSizeF(qMax(6.0, smallFont.pointSizeF() * 0.78));
    QFontMetrics fmSmall(smallFont);
    QFontMetrics fm(option.font);
    float s = ThemeManager::instance().scaleF();
    int padding = qRound(8 * s);
    int w = qMax(fm.horizontalAdvance("-9999.9") + padding,
                 fmSmall.horizontalAdvance("-9999.9, -9999.9") + padding);

    return QSize(w, fm.height() * 2 + qRound(6 * s));
}
