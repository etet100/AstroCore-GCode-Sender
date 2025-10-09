#ifndef GLCONTAINER_H
#define GLCONTAINER_H

#ifdef USE_GLWINDOW
#include <QWidget>
#include <QOpenGLWindow>
#endif
#include "glwidget.h"
#include "drawers/shaderdrawable.h"

#ifdef USE_GLWINDOW
class GLContainer : public QWidget
{
    Q_OBJECT

    public:
        GLContainer(QWidget *parent);
        void addDrawable(ShaderDrawable *drawable);
        GLContainer &operator<<(ShaderDrawable *drawable);

        void updateExtremes(ShaderDrawable *drawable);
        void fitDrawable(ShaderDrawable *drawable = NULL);
        bool antialiasing() const;
        void setAntialiasing(bool antialiasing);

        QTime spendTime() const;
        void setSpendTime(const QTime &spendTime);

        QTime estimatedTime() const;
        void setEstimatedTime(const QTime &estimatedTime);

        double lineWidth() const;
        void setLineWidth(double lineWidth);

        void setIsometricView();
        void setTopView();
        void setBottomView();
        void setFrontView();
        void setBackView();
        void setLeftView();
        void setRightView();
        void toggleProjectionType();
        void toggleRotationCube();

        int fps();
        void setFps(int fps);

        QString parserState() const;
        void setParserState(const QString &parserState);

        QString bufferState() const;
        void setBufferState(const QString &bufferState);

        bool zBuffer() const;
        void setZBuffer(bool zBuffer);

        double fov();
        void setFov(double fov);

        double nearPlane();
        void setNearPlane(double plane);

        double farPlane();
        void setFarPlane(double plane);

        bool updatesEnabled() const;
        void setUpdatesEnabled(bool updatesEnabled);

        bool msaa() const;
        void setMsaa(bool msaa);

        QColor colorBackground() const;
        void setColorBackground(const QColor &colorBackground);

        QColor colorText() const;
        void setColorText(const QColor &colorText);

        double pointSize() const;
        void setPointSize(double pointSize);

        bool vsync() const;
        void setVsync(bool vsync);

        QString speedState() const;
        void setSpeedState(const QString &speedState);

        QString pinState() const;
        void setPinState(const QString &pinState);
        
        void updateDrawer(ShaderDrawable *);
    protected:
        void showEvent(QShowEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    private:
        GLWidget* m_glWidget;

    signals:
        void rotationChanged();
        void cursorPosChanged(QPointF);
        void resized();
};
#else
class GLContainer : public GLWidget
{
    public:
        GLContainer(QWidget *parent) : GLWidget(parent) {}
};
#endif

#endif // GLCONTAINER_H
