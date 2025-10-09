// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#ifndef GLWIDGET_H
#define GLWIDGET_H

#ifndef GLES
#include <QGLWidget>
#else
#include <QOpenGLWidget>
#include <QOpenGLWindow>
#endif

#include <QOpenGLFunctions_3_0>
#include <QTimer>
#include <QTime>
#include "drawers/shaderdrawable.h"
#include "drawers/cubedrawer.h"
#include "glpalette.h"
#include "glzminmax.h"

#ifdef GLES
#ifdef USE_GLWINDOW
class GLWidget : public QOpenGLWindow, protected QOpenGLFunctions_3_0
#else
class GLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_0
#endif
#else
class GLWidget : public QGLWidget, protected QOpenGLFunctions
#endif
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    ~GLWidget();

    void addDrawable(ShaderDrawable *drawable);
    GLWidget &operator<<(ShaderDrawable *drawable);

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

    void setOffset(double val) {
        m_offset = val;
    }

signals:
    void rotationChanged();
    void cursorPosChanged(QPointF);
    void resized();

public slots:

private slots:
    void onFramesTimer();
    void onAnimation();

private:
    double m_xRot, m_yRot, m_xLastRot, m_yLastRot;
    QVector3D m_lookAt;
    QVector3D m_eye;
    bool m_perspective;
    QPoint m_lastPos;
    double m_zoomDistance;
    double m_fov, m_near, m_far;
    double m_xMin, m_xMax, m_yMin, m_yMax, m_zMin, m_zMax, m_xSize, m_ySize, m_zSize;
    double m_lineWidth;
    double m_pointSize;
    double m_offset;
    bool m_antialiasing;
    bool m_msaa;
    bool m_vsync;
    bool m_zBuffer;
    bool m_rotationCube = true;
    int m_frames;
    int m_fps;
    int m_targetFps;
    int m_animationFrame;
    QTime m_spendTime;
    QTime m_estimatedTime;
    QBasicTimer m_timerPaint;
    double m_xRotTarget, m_yRotTarget;
    double m_xRotStored, m_yRotStored;
    bool m_animateView;
    QString m_parserState;
    QString m_speedState;
    QString m_pinState;
    QString m_bufferState;
    QPointF m_bottomSurfaceCursorPos;
    bool m_updatesEnabled;

    double normalizeAngle(double angle);
    void animate();
    void stopAnimation();

    QList<ShaderDrawable*> m_shaderDrawables;
    QOpenGLShaderProgram *m_defaultShaderProgram;
    QOpenGLShaderProgram *m_gcodeShaderProgram;
    QOpenGLShaderProgram *m_copyProgram;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    GLPalette m_palette;
    CubeDrawer m_cubeDrawer;

    QColor m_colorBackground;
    QColor m_colorText;

    QPointF getClickPositionOnXYPlane(QVector2D mouseClickPosition, QMatrix4x4 projectionMatrix, QMatrix4x4 viewMatrix);
    void drawText(QPainter &painter, QPoint &pos, QString text, int lineHeight, Qt::AlignmentFlag align = Qt::AlignLeft);
    void drawTexts(QPainter &painter, QPoint &pos, QStringList texts, int lineHeight);

    void initializeDebugLogger();
    void initializeGL() override;

protected:
    void resizeGL(int width, int height) override;
    void updateProjection();
    void updateView();

#ifdef GLES
    void paintGL() override;
#else
    void paintEvent(QPaintEvent *pe);
#endif

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void wheelEvent(QWheelEvent *we) override;

    void timerEvent(QTimerEvent *) override;
};

#endif // GLWIDGET_H
