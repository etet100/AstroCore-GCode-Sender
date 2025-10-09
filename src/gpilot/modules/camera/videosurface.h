#ifndef VIDEOSURFACE_H
#define VIDEOSURFACE_H

/*
#include <QAbstractVideoSurface>

class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
public:
    VideoSurface();
    ~VideoSurface();

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(
            QAbstractVideoBuffer::HandleType type = QAbstractVideoBuffer::NoHandle) const override;

    bool present(const QVideoFrame &frame) override;
    void setDisplaySize(int width, int height);
    QSize realSize() const { return m_realSize; }

private:
    QSize m_maxSize;
    QSize m_realSize;

signals:
    void image(const QPixmap &);
    void realSizeChanged(const QSize &);

};
*/

#endif // VIDEOSURFACE_H
