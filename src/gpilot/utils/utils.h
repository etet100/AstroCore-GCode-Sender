// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef UTIL
#define UTIL

#include <QColor>
#include <QIcon>
#include <QImage>
#include <QAbstractButton>
#include <QVector3D>
#include <QEventLoop>
#include <QTimer>
#include <QDockWidget>
#include <QStyle>

class Utils
{
    public:
        static void positionDialog(QWidget *widget, QRect geometry, bool maximized);
        static QRect getDialogGeometry(QWidget *widget);
        static bool pointInTriangle(QPoint p, QPoint p0, QPoint p1, QPoint p2);
        static bool triangleDir(QPoint p0, QPoint p1, QPoint p2);
        static void setVisualMode(QWidget *widget, bool dark);
        static void setDockableLocked(QDockWidget *widget, bool locked);

        static double nMin(double v1, double v2)
        {
            if (!qIsNaN(v1) && !qIsNaN(v2)) return qMin<double>(v1, v2);
            else if (!qIsNaN(v1)) return v1;
            else if (!qIsNaN(v2)) return v2;
            else return qQNaN();
        }

        static double nMax(double v1, double v2)
        {
            if (!qIsNaN(v1) && !qIsNaN(v2)) return qMax<double>(v1, v2);
            else if (!qIsNaN(v1)) return v1;
            else if (!qIsNaN(v2)) return v2;
            else return qQNaN();
        }

        static QVector3D colorToVector(QColor color)
        {
            return QVector3D(255.0 / color.redF(), 255.0 / color.greenF(), 255.0 / color.blueF());
        }

        static void waitEvents(int ms)
        {
            QEventLoop loop;

            QTimer::singleShot(ms, &loop, SLOT(quit()));
            loop.exec();
        }

        static QIcon invertIconColors(QIcon icon)
        {
            QImage img = icon.pixmap(icon.actualSize(QSize(64, 64))).toImage();
            img.invertPixels();

            return QIcon(QPixmap::fromImage(img));
        }

        static void invertButtonIconColors(QAbstractButton *button)
        {
            button->setIcon(invertIconColors(button->icon()));
        }

        static bool isHeightmapFile(QString fileName)
        {
            return fileName.endsWith(".map", Qt::CaseInsensitive);
        }

        static bool isGCodeFile(QString fileName)
        {
            return fileName.endsWith(".txt", Qt::CaseInsensitive)
                   || fileName.endsWith(".nc", Qt::CaseInsensitive)
                   || fileName.endsWith(".ncc", Qt::CaseInsensitive)
                   || fileName.endsWith(".ngc", Qt::CaseInsensitive)
                   || fileName.endsWith(".tap", Qt::CaseInsensitive)
                   || fileName.endsWith(".gc", Qt::CaseInsensitive)
                   || fileName.endsWith(".gcode", Qt::CaseInsensitive);
        }

        static void refreshStyle(QWidget* widget)
        {
            widget->style()->unpolish(widget);
            widget->style()->polish(widget);
        }
};

#endif // UTIL

