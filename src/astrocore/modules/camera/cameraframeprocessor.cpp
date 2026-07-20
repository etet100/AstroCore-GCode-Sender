// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#include "cameraframeprocessor.h"
#include <QPainter>
#include <QDebug>

CameraFrameProcessor::CameraFrameProcessor(QObject *parent)
    : QObject(parent)
{}

void CameraFrameProcessor::setVideoSinks(QVideoSink *inputSink, QVideoSink *outputSink)
{
    if (m_inputSink != inputSink) {
        if (m_inputSink != nullptr) {
            disconnect(m_inputSink, &QVideoSink::videoFrameChanged, this, &CameraFrameProcessor::processFrame);
        }
        m_inputSink = inputSink;
        if (m_inputSink != nullptr) {
            connect(m_inputSink, &QVideoSink::videoFrameChanged, this, &CameraFrameProcessor::processFrame);
        }
    }

    m_outputSink = outputSink;
}

void CameraFrameProcessor::processFrame(const QVideoFrame &frame)
{
    if (m_style == CrosshairStyle::None) {
        m_outputSink->setVideoFrame(frame);

        return;
    }

    QImage image = frame.toImage();

    QPainter painter(&image);
    if (!painter.isActive()) {
        return;
    }

    drawCrosshair(painter, image.size());

    QVideoFrame frameCopy(image);

    m_outputSink->setVideoFrame(frameCopy);
}

void CameraFrameProcessor::drawCrosshair(QPainter &painter, const QSize &size)
{
    const int cx = size.width() / 2;
    const int cy = size.height() / 2;

    // scale line width and center gap to the frame size so the crosshair
    // stays visible on both low- and high-resolution cameras
    const int penWidth = qMax(2, size.height() / 300);
    const int gap = qMax(6, size.height() / 40);

    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(Qt::red);
    pen.setWidth(penWidth);
    painter.setPen(pen);
    painter.setBrush(Qt::NoBrush);

    switch (m_style) {
        case CrosshairStyle::Cross:
            // full lines crossing the whole frame
            painter.drawLine(0, cy, size.width(), cy);
            painter.drawLine(cx, 0, cx, size.height());
            break;

        case CrosshairStyle::CrossGap:
            // lines with a gap in the middle and a circle at the center
            painter.drawLine(0, cy, cx - gap, cy);
            painter.drawLine(cx + gap, cy, size.width(), cy);
            painter.drawLine(cx, 0, cx, cy - gap);
            painter.drawLine(cx, cy + gap, cx, size.height());
            painter.drawEllipse(QPoint(cx, cy), gap, gap);
            break;

        case CrosshairStyle::Dot:
            // single filled dot marking the target point
            painter.setBrush(Qt::red);
            painter.drawEllipse(QPoint(cx, cy), penWidth * 2, penWidth * 2);
            break;

        case CrosshairStyle::CircleTicks: {
            // center circle with short ticks pointing inward
            const int radius = gap * 2;
            const int tick = gap;
            painter.drawEllipse(QPoint(cx, cy), radius, radius);
            painter.drawLine(cx, cy - radius - tick, cx, cy - radius);
            painter.drawLine(cx, cy + radius, cx, cy + radius + tick);
            painter.drawLine(cx - radius - tick, cy, cx - radius, cy);
            painter.drawLine(cx + radius, cy, cx + radius + tick, cy);
            break;
        }

        case CrosshairStyle::None:
            break;
    }
}

void CameraFrameProcessor::setCrosshairStyle(CrosshairStyle style)
{
    m_style = style;
}
