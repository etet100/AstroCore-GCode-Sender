#ifndef VIEWFINDER_H
#define VIEWFINDER_H

#include <QLabel>
#include "videosurface.h"

class ViewFinder : public QLabel
{
    Q_OBJECT

public:
    ViewFinder(VideoSurface *videoSurface, QWidget *parent = nullptr);
    ~ViewFinder();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void paintEvent(QPaintEvent *event) override;
    VideoSurface *m_videoSurface;
};

#endif // VIEWFINDER_H
