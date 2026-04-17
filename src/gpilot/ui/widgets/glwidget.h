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
#include "ui/drawers/shaderdrawable.h"
#include "ui/drawers/idrawable.h"
#include "ui/drawers/cubedrawer.h"
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

    enum class ViewMode {
        Perspective,
        Orthogonal,
        View2D
    };

    void addDrawable(IDrawable *drawable);
    GLWidget &operator<<(IDrawable *drawable);

    void updateExtremes(IDrawable *drawable);
    // Set default extremes (0,0,0) - (50,50,10) - to avoid problems with 0 size drawable
    void setDefaultExtemes();
    void fitDrawable(IDrawable *drawable = nullptr);
    void setAntialiasing(bool antialiasing);
    void setSpendTime(const QTime &spendTime);
    void setEstimatedTime(const QTime &estimatedTime);
    void setLineWidth(double lineWidth);
    void setIsometricView();
    void setTopView();
    void setBottomView();
    void setFrontView();
    void setBackView();
    void setLeftView();
    void setRightView();
    void setBackLeftView();
    void setBackRightView();
    void setBackBottomView();
    void setLeftTopView();
    void setFrontLeftView();
    void setLeftBottomView();
    void setFrontRightView();
    void setFrontBottomView();
    void setRightBottomView();
    void setRightTopView();
    void setFrontTopView();
    void setBackTopView();
    void toggleProjectionType();
    void toggleRotationCube();
    void toggleLight();
    void setViewMode(ViewMode mode);
    void set2DView();
    void setFps(int fps);
    void setParserState(const QString &parserState);
    void setBufferState(const QString &bufferState);
    void setLightCenter(const QVector3D &lightCenter);
    void setZBuffer(bool zBuffer);
    void setFov(double fov);
    void setNearPlane(double plane);
    void setFarPlane(double plane);
    void setUpdatesEnabled(bool updatesEnabled);
    void setMsaa(bool msaa);
    void setColorBackground(const QColor &colorBackground);
    void setColorText(const QColor &colorText);
    void setPointSize(double pointSize);
    void setVsync(bool vsync);
    void setSpeedState(const QString &speedState);
    void setPinState(const QString &pinState);
    void updateDrawer(ShaderDrawable *);

    void setOffset(double val) {
        m_offset = val;
    }

    QMatrix4x4& viewMatrix() { return m_viewMatrix; }
    QMatrix4x4& projectionMatrix() { return m_projectionMatrix; }
    QVector3D& lightPos() { return m_lightPos; }

signals:
    void rotated();
    void cursorPosChanged(QPointF);
    void mouseDoubleClicked(QPoint);
    void resized();
    void entered();
    void left();
    void goToCursor(QPointF);
    void zoomChanged(double);
    void mouseMoved(QPoint);
    void viewModeChanged(ViewMode mode);
    void viewParametersChanged();

private slots:
    void onFramesTimer();
    void onAnimation();
    void onViewChangeTimerTimeout();

private:
    double m_xRot, m_yRot, m_xLastRot, m_yLastRot;
    QVector3D m_lookAt;
    QVector3D m_eye;
    ViewMode m_mode;
    QPoint m_lastPos;
    double m_zoomDistance;
    double m_fov, m_near, m_far;
    double m_xMin = 0, m_xMax = 0, m_yMin = 0, m_yMax = 0, m_zMin = 0, m_zMax = 0, m_xSize = 0, m_ySize = 0, m_zSize = 0;
    double m_lineWidth;
    double m_offset;
    bool m_error = false;
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
    QTimer m_viewChangeTimer;
    double m_xRotTarget, m_yRotTarget;
    double m_xRotStored, m_yRotStored;
    QString m_parserState;
    QString m_speedState;
    QString m_pinState;
    QString m_bufferState;
    QPointF m_bottomSurfaceCursorPos;
    QVector3D m_lightCenter;
    QVector3D m_lightPos;
    bool m_updatesEnabled;
    bool m_viewChanged;

    double normalizeAngle(double angle);
    void animate();
    void stopAnimation();

    QList<IDrawable*> m_shaderDrawables;
    QOpenGLShaderProgram *m_defaultShaderProgram;
    QOpenGLShaderProgram *m_gcodeShaderProgram;
    QOpenGLShaderProgram *m_billboardShaderProgram;
    QOpenGLShaderProgram *m_copyProgram;
    QMatrix4x4 m_projectionMatrix;
    QMatrix4x4 m_viewMatrix;
    GLPalette m_palette;
    CubeDrawer m_cubeDrawer;
    bool m_light = false;
    QColor m_colorBackground;
    QColor m_colorText;
    QTimer m_animationTimer;

    QPointF calcPositionOnXYPlane(QPoint mouseClickPosition);
    void drawText(QPainter &painter, QPoint &pos, QString text, int lineHeight, Qt::AlignmentFlag align = Qt::AlignLeft);
    void drawTexts(QPainter &painter, QPoint &pos, QStringList texts, int lineHeight);

    void initializeDebugLogger();
    void initializeGL() override;

    void emitZoomChanged();

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
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
#ifndef USE_GLWINDOW
    void leaveEvent(QEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
#endif
    void wheelEvent(QWheelEvent *we) override;
};

#endif // GLWIDGET_H
