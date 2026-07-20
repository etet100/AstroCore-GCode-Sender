// This file is a part of "G-Pilot GCode Sender" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich
// Copyright 2024 BTS

#ifndef CAMERAFRAMEPROCESSOR_H
#define CAMERAFRAMEPROCESSOR_H

#include <QVideoSink>
#include <QVideoFrame>

class QPainter;

class CameraFrameProcessor : public QObject
{
    Q_OBJECT

    public:
        enum class CrosshairStyle {
            None,
            Cross,
            CrossGap,
            Dot,
            CircleTicks
        };
        Q_ENUM(CrosshairStyle)

        CameraFrameProcessor(QObject *parent);
        void setVideoSinks(QVideoSink *inputSink, QVideoSink *outputSink);
        void setCrosshairStyle(CrosshairStyle style);
        CrosshairStyle crosshairStyle() const { return m_style; }

    signals:
        void videoSinkChanged();

    public slots:
        void processFrame(const QVideoFrame &frame);

    private:
        void drawCrosshair(QPainter &painter, const QSize &size);

        QVideoSink *m_inputSink = nullptr;
        QVideoSink *m_outputSink = nullptr;
        CrosshairStyle m_style = CrosshairStyle::CrossGap;
};

#endif // CAMERAFRAMEPROCESSOR_H
