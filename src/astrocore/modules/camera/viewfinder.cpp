#include "viewfinder.h"
#include <QPainter>
#include <QPaintEvent>
#include <QImage>

ViewFinder::ViewFinder(VideoSurface *videoSurface, QWidget* parent)
    : QLabel(parent), m_videoSurface(videoSurface)
{
    connect(m_videoSurface, &VideoSurface::realSizeChanged, [this](const QSize &size) {
        this->setMaximumSize(16777215, size.height());
    });
}

ViewFinder::~ViewFinder()
{
}

void ViewFinder::resizeEvent(QResizeEvent *event)
{
    m_videoSurface->setDisplaySize(event->size().width(), event->size().height());
    QWidget::resizeEvent(event);
}

void ViewFinder::paintEvent(QPaintEvent* event)
{
   QLabel::paintEvent(event);

   const int wh = this->width() / 2;
   const int hh = this->height() / 2;


   QPainter painter(this);
   QPen pen = painter.pen();

   pen.setColor(Qt::red);
   pen.setWidth(3);
   painter.setPen(pen);

   painter.drawLine(wh, 0, wh, this->height());
   painter.drawLine(0, hh, this->width(), hh);
}
