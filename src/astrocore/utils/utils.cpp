#include "utils.h"
#include <QScreen>
#include <QStyle>
#include <QApplication>

// info about sizing https://doc.qt.io/qt-6/application-windows.html
void Utils::positionDialog(QWidget *widget, QRect frameGeometry, bool maximized)
{
    QSize screenSize = widget->screen()->availableSize();
    QSize geometrySize = frameGeometry.size();

    // if geometry is not set, center it
    if (frameGeometry.x() == -1) {
        frameGeometry.setLeft(screenSize.width() / 2 - frameGeometry.width() / 2);
        frameGeometry.setWidth(geometrySize.width());
    }
    if (frameGeometry.y() == -1) {
        frameGeometry.setTop(screenSize.height() / 2 - frameGeometry.height() / 2);
        frameGeometry.setHeight(geometrySize.height());
    }

    // prevent hiding window out of screen
    if (frameGeometry.x() < 0) {
        int t = frameGeometry.width();
        frameGeometry.setX(0);
        frameGeometry.setWidth(t);
    }
    if (frameGeometry.y() < 0) {
        int t = frameGeometry.height();
        frameGeometry.setY(0);
        frameGeometry.setHeight(t);
    }
    if (frameGeometry.right() >= screenSize.width()) {
        frameGeometry.setRight(screenSize.width());
    }
    if (frameGeometry.bottom() >= screenSize.height()) {
        frameGeometry.setBottom(screenSize.height());
    }

    if (frameGeometry.width() == -1 || frameGeometry.x() + frameGeometry.width() > screenSize.width()) {
        frameGeometry.setWidth(screenSize.width() - frameGeometry.x());
    }
    if (frameGeometry.height() == -1 || frameGeometry.y() + frameGeometry.height() > screenSize.height()) {
        frameGeometry.setHeight(screenSize.height() - frameGeometry.y());
    }

    QSize frameOnlySize = widget->frameSize() - widget->size();
    QSize clientSize = frameGeometry.size() - frameOnlySize;
    widget->move(frameGeometry.x(), frameGeometry.y());
    widget->resize(clientSize);

    if (maximized) {
        widget->showMaximized();
    }
}

QRect Utils::getDialogGeometry(QWidget *widget)
{
    QRect geometry = widget->geometry();
    geometry.setWidth(widget->width() + widget->frameSize().width());
    geometry.setHeight(widget->height() + widget->frameSize().height());
    geometry.moveTopLeft(widget->mapToGlobal(geometry.topLeft()));

    return geometry;
}

bool Utils::pointInTriangle(QPoint p, QPoint p0, QPoint p1, QPoint p2)
{
    int dX = p.x()-p2.x();
    int dY = p.y()-p2.y();
    int dX21 = p2.x()-p1.x();
    int dY12 = p1.y()-p2.y();
    int d = dY12*(p0.x()-p2.x()) + dX21*(p0.y()-p2.y());
    int s = dY12*dX + dX21*dY;
    int t = (p2.y()-p0.y())*dX + (p0.x()-p2.x())*dY;

    if (d<0) {
        return s<=0 && t<=0 && s+t>=d;
    }

    return s>=0 && t>=0 && s+t<=d;
}

bool Utils::triangleDir(QPoint p0, QPoint p1, QPoint p2)
{
    return (p0.x() - p2.x()) * (p1.y() - p2.y()) - (p1.x() - p2.x()) * (p0.y() - p2.y()) < 0;
}

void Utils::setVisualMode(QWidget *widget, bool dark)
{
    widget->setProperty("mode", dark ? "dark" : "light");
    Utils::refreshStyle(widget);
}

void Utils::setDockableLocked(QDockWidget *widget, bool locked)
{
    widget->setFeatures(locked ? QDockWidget::NoDockWidgetFeatures
                               : QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable
                                     | QDockWidget::DockWidgetFloatable);
}
