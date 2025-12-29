// This file is a part of "Candle" application.
// Copyright 2015-2021 Hayrullin Denis Ravilevich

#include "glwidget.h"
#include "ui/drawers/gcodedrawer.h"
#include <QDebug>
#include <QtWidgets>
#include <QPainter>
#include <QEasingCurve>
#include <QOpenGLDebugLogger>
#include "ui/utils/thememanager.h"

#ifdef GLES
//#include <GLES/gl.h>
#endif

//1 = old/classic, 2 = new
#define NAV_MODE 2
#define ZOOMSTEP 1.1
#define DEFAULT_ZOOM 200
#if NAV_MODE == 1
    #define MIN_ZOOM  0.2
#endif
#if NAV_MODE == 2
    #define MIN_ZOOM  10.0
    #define MAX_ZOOM  10000.0
#endif
#define ONE_DEG_IN_RAD 0.0174533
// what is this value? oryginally was 1.25
#define MAGIC_ZOOM_MULTIPLIER 1.9

#ifdef GLES
#ifdef USE_GLWINDOW
GLWidget::GLWidget(QWidget *parent) : QOpenGLWindow(), m_defaultShaderProgram(0), m_gcodeShaderProgram(0), m_palette()
#else
GLWidget::GLWidget(QWidget *parent) : QOpenGLWidget(parent), m_defaultShaderProgram(0), m_gcodeShaderProgram(0), m_palette()
#endif
#else
GLWidget::GLWidget(QWidget *parent) : QGLWidget(parent), m_shaderProgram(0)
#endif
{
    m_frames = 0;
    m_fps = 0;

    m_animateView = false;
    m_updatesEnabled = false;

    m_xRot = m_xRotTarget = 35.264;
    m_yRot = m_yRotTarget = 0;// m_yRot > 180 ? 405 : 45;

    m_zoomDistance = DEFAULT_ZOOM;

    m_lookAt = QVector3D(0, 0, 0);
    m_eye = QVector3D(0, 0, -50);

    m_mode = ViewMode::Perspective;

    m_fov = 30;
    m_near = 0.5;
    m_far = 5000.0;

    m_xMin = 0;
    m_xMax = 0;
    m_yMin = 0;
    m_yMax = 0;
    m_zMin = 0;
    m_zMax = 0;
    m_xSize = 0;
    m_ySize = 0;
    m_zSize = 0;

    updateProjection();
    updateView();

    m_spendTime.setHMS(0, 0, 0);
    m_estimatedTime.setHMS(0, 0, 0);

    m_vsync = false;
    m_targetFps = 60;

    QTimer::singleShot(1000, this, SLOT(onFramesTimer()));

    // required for mouseMoveEvent to be called without clicking
    setMouseTracking(true);

    // enable antialiasing
    QSurfaceFormat sf = format();
    sf.setSamples(16);
    setFormat(sf);
}

GLWidget::~GLWidget()
{
    if (m_defaultShaderProgram) {
        delete m_defaultShaderProgram;
    }
    if (m_gcodeShaderProgram) {
        delete m_gcodeShaderProgram;
    }
}

void GLWidget::addDrawable(ShaderDrawable *drawable)
{
    m_shaderDrawables.append(drawable);
}

GLWidget& GLWidget::operator<<(ShaderDrawable *drawable)
{
    addDrawable(drawable);

    return *this;
}

void GLWidget::emitZoomChanged()
{
    if (m_mode == ViewMode::Perspective) {
        // distance between eye and origin (0,0,0)
        emit zoomChanged(m_eye.length() / 100.0);
    } else {
        emit zoomChanged(m_zoomDistance / 100.0);
    }
}

void GLWidget::fitDrawable(ShaderDrawable *drawable)
{
    stopAnimation();

    if (drawable != nullptr) {
        updateExtremes(drawable);

        // Calculate center of drawable in world space
        QVector3D center(
            (m_xMin + m_xMax) * 0.5f,
            (m_yMin + m_yMax) * 0.5f,
            (m_zMin + m_zMax) * 0.5f
        );
        m_lookAt = center;

        // Calculate camera basis vectors (Z-up system) to determine object orientation relative to camera
        float pitch = qDegreesToRadians((float)m_xRot);
        float yaw = qDegreesToRadians((float)m_yRot);
        float cosPitch = cos(pitch);
        float sinPitch = sin(pitch);
        float cosYaw = cos(yaw);
        float sinYaw = sin(yaw);

        // Camera Z axis (pointing towards viewer)
        QVector3D camZ(sinYaw * cosPitch, -cosYaw * cosPitch, sinPitch);
        camZ.normalize();

        // Camera Y axis (Up)
        QVector3D camY(-sinYaw * sinPitch, cosYaw * sinPitch, cosPitch);
        if (qAbs(cosPitch) < 0.001f) {
            camY = QVector3D(sinYaw, cosYaw, 0);
            if (pitch < 0) camY = -camY;
        }
        camY.normalize();

        // Camera X axis (Right)
        QVector3D camX = QVector3D::crossProduct(camY, camZ).normalized();

        // Calculate extents of the AABB projected onto the camera plane
        // We use the "Separating Axis Theorem" logic here.
        // The extent of the AABB along a vector V is sum(|halfSize_i * dot(axis_i, V)|)
        float dx = m_xSize * 0.5f;
        float dy = m_ySize * 0.5f;
        float dz = m_zSize * 0.5f;

        float maxProjX = qAbs(dx * camX.x()) + qAbs(dy * camX.y()) + qAbs(dz * camX.z());
        float maxProjY = qAbs(dx * camY.x()) + qAbs(dy * camY.y()) + qAbs(dz * camY.z());
        float maxProjZ = qAbs(dx * camZ.x()) + qAbs(dy * camZ.y()) + qAbs(dz * camZ.z());

        if (maxProjX > 0.0f || maxProjY > 0.0f) {
            if (m_mode == ViewMode::Perspective) {
                float aspectRatio = width() / float(height() ? height() : 1);
                float fovRad = qDegreesToRadians((float)m_fov);

                // Calculate required distance
                // For perspective, we must account for the fact that the front of the object
                // is closer to the camera and thus appears larger.
                // We need the frustum to be large enough at the front of the object (dist - maxProjZ).
                float distY = maxProjY / tan(fovRad * 0.5f);
                float distX = maxProjX / (tan(fovRad * 0.5f) * aspectRatio);

                // Add maxProjZ to distance to ensure front face fits
                m_zoomDistance = qMax(distX, distY) + maxProjZ;

                // Use minimal margin (1%)
                // m_zoomDistance *= 1.01f;

                // Ensure we don't clip the front of the object with near plane
                m_zoomDistance = qMax(m_zoomDistance, maxProjZ + m_near * 1.1f);

                qDebug() << "[GLWidget] Perspective fit (Tight+Depth): projX=" << maxProjX << "projY=" << maxProjY
                         << "dist=" << m_zoomDistance;
            } else {
                // For orthographic projection
                float aspectRatio = width() / float(height() ? height() : 1);

                float sizeY = maxProjY;
                float sizeX = maxProjX / aspectRatio;

                // Use minimal margin (1%)
                m_zoomDistance = qMax(sizeX, sizeY) * 1.01f;

                // Ensure we don't clip the front of the object with near plane
                m_zoomDistance = qMax(m_zoomDistance, maxProjZ + m_near * 1.1f);

                qDebug() << "[GLWidget] Ortho fit (Tight): projX=" << maxProjX << "projY=" << maxProjY
                         << "orthoSize=" << m_zoomDistance;
            }
        } else {
            m_zoomDistance = DEFAULT_ZOOM;
        }        qDebug() << "[GLWidget] FitDrawable: center=" << center
                 << "size=" << m_xSize << m_ySize << m_zSize
                 << "finalZoom=" << m_zoomDistance;
    } else {
        m_lookAt = QVector3D(0, 0, 0);

        m_xMin = 0;
        m_xMax = 0;
        m_yMin = 0;
        m_yMax = 0;
        m_zMin = 0;
        m_zMax = 0;

        m_xSize = 0;
        m_ySize = 0;
        m_zSize = 0;
    }

    updateProjection();
    updateView();
    emitZoomChanged();
}

void GLWidget::updateExtremes(ShaderDrawable *drawable)
{
    QVector3D minExtremes = drawable->minimumExtremes();
    QVector3D maxExtremes = drawable->maximumExtremes();

    m_xMin = !qIsNaN(minExtremes.x()) ? minExtremes.x() : 0;
    m_xMax = !qIsNaN(maxExtremes.x()) ? maxExtremes.x() : 0;
    m_yMin = !qIsNaN(minExtremes.y()) ? minExtremes.y() : 0;
    m_yMax = !qIsNaN(maxExtremes.y()) ? maxExtremes.y() : 0;
    m_zMin = !qIsNaN(minExtremes.z()) ? minExtremes.z() : 0;
    m_zMax = !qIsNaN(maxExtremes.z()) ? maxExtremes.z() : 0;

    m_xSize = m_xMax - m_xMin;
    m_ySize = m_yMax - m_yMin;
    m_zSize = m_zMax - m_zMin;

    qDebug() << "[GLWidget] Extremes updated: "
             << "X:" << m_xMin << "..." << m_xMax
             << "Y:" << m_yMin << "..." << m_yMax
             << "Z:" << m_zMin << "..." << m_zMax
             << "Sizes:"
             << m_xSize << m_ySize << m_zSize;
}

bool GLWidget::antialiasing() const
{
    return m_antialiasing;
}

void GLWidget::setAntialiasing(bool antialiasing)
{
    m_antialiasing = antialiasing;
}

void GLWidget::onFramesTimer()
{
    m_fps = m_frames;
    m_frames = 0;

    QTimer::singleShot(1000, this, SLOT(onFramesTimer()));
}

void GLWidget::onAnimation()
{
    double t = (double) m_animationFrame++ / (m_fps * 0.2);

    if (t >= 1) {
        stopAnimation();
    }

    QEasingCurve ec(QEasingCurve::OutExpo);
    double val = ec.valueForProgress(t);

    m_xRot = m_xRotStored + double(m_xRotTarget - m_xRotStored) * val;
    m_yRot = m_yRotStored + double(m_yRotTarget - m_yRotStored) * val;

    updateView();
}

QString GLWidget::pinState() const
{
    return m_pinState;
}

void GLWidget::setPinState(const QString &pinState)
{
    m_pinState = pinState;
}

void GLWidget::updateDrawer(ShaderDrawable *drawer)
{
    drawer->update();
    drawer->updateData(m_palette);
}

QString GLWidget::speedState() const
{
    return m_speedState;
}

void GLWidget::setSpeedState(const QString &additionalStatus)
{
    m_speedState = additionalStatus;
}

bool GLWidget::vsync() const
{
    return m_vsync;
}

void GLWidget::setVsync(bool vsync)
{
    m_vsync = vsync;
}

bool GLWidget::msaa() const
{
    return m_msaa;
}

void GLWidget::setMsaa(bool msaa)
{
    m_msaa = msaa;
}

bool GLWidget::updatesEnabled() const
{
    return m_updatesEnabled;
}

void GLWidget::setUpdatesEnabled(bool updatesEnabled)
{
    m_updatesEnabled = updatesEnabled;

    if (updatesEnabled) {
        m_timerPaint.start(m_vsync ? 0 : 1000 / m_targetFps, Qt::PreciseTimer, this);
    } else {
        m_timerPaint.stop();
    }
}

bool GLWidget::zBuffer() const
{
    return m_zBuffer;
}

void GLWidget::setZBuffer(bool zBuffer)
{
    m_zBuffer = zBuffer;
}

double GLWidget::fov() {
    return m_fov;
}

void GLWidget::setFov(double fov) {
    m_fov = fov;
    updateProjection();
}

double GLWidget::nearPlane() {
    return m_near;
}

void GLWidget::setNearPlane(double plane) {
    m_near = plane;
    updateProjection();
}

double GLWidget::farPlane() {
    return m_far;
}

void GLWidget::setFarPlane(double plane) {
    m_far = plane;
    updateProjection();
}

QString GLWidget::bufferState() const
{
    return m_bufferState;
}

void GLWidget::setBufferState(const QString &bufferState)
{
    m_bufferState = bufferState;
}

QString GLWidget::parserState() const
{
    return m_parserState;
}

void GLWidget::setParserState(const QString &parserState)
{
    m_parserState = parserState;
}


double GLWidget::lineWidth() const
{
    return m_lineWidth;
}

void GLWidget::setLineWidth(double lineWidth)
{
    m_lineWidth = lineWidth;
}

void GLWidget::setTopView()
{
    m_xRotTarget = 90;
    m_yRotTarget = m_yRot > 180 ? 360 : 0;
    animate();
}

void GLWidget::setBottomView()
{
    m_xRotTarget = -90;
    m_yRotTarget = m_yRot > 180 ? 360 : 0;
    animate();
}

void GLWidget::setFrontView()
{
    m_xRotTarget = 0;
    m_yRotTarget = m_yRot > 180 ? 360 : 0;
    animate();
}

void GLWidget::setBackView()
{
    m_xRotTarget = 0;
    m_yRotTarget = 180;
    animate();
}

void GLWidget::setRightView()
{
    m_xRotTarget = 0;
    m_yRotTarget = m_yRot > 270 ? 450 : 90;
    animate();
}

void GLWidget::setLeftView()
{
    m_xRotTarget = 0;
    m_yRotTarget = m_yRot > 90 ? 270 : -90;
    animate();
}

int GLWidget::fps()
{
    return m_targetFps;
}

void GLWidget::toggleProjectionType() {
    switch (m_mode) {
        case ViewMode::View2D:
        case ViewMode::Perspective:
            m_mode = ViewMode::Orthogonal;
            break;
        case ViewMode::Orthogonal:
            m_mode = ViewMode::Perspective;
            break;
    }
    updateProjection();
    updateView();
    emit viewModeChanged(m_mode);
}

void GLWidget::toggleRotationCube()
{
    m_rotationCube = !m_rotationCube;
}

void GLWidget::setIsometricView()
{
    m_mode = ViewMode::Orthogonal;
    updateProjection();
    m_xRotTarget = 35.264;
    m_yRotTarget = m_yRot > 180 ? 405 : 45;
    animate();
    emit viewModeChanged(m_mode);
}

void GLWidget::animate()
{
    m_xRotStored = m_xRot;
    m_yRotStored = m_yRot;
    m_animationFrame = 0;
    m_animateView = true;
}

void GLWidget::stopAnimation()
{
    m_animateView = false;
}

GLWidget::ViewMode GLWidget::viewMode() const
{
    return m_mode;
}

void GLWidget::setViewMode(ViewMode mode)
{
    m_mode = mode;
    if (m_mode == ViewMode::View2D) {
        set2DView();
    }
    updateProjection();
    updateView();
    emit viewModeChanged(m_mode);
}

void GLWidget::set2DView()
{
    // Lock rotation to top view
    m_xRotTarget = 90;
    m_yRotTarget = m_yRot > 180 ? 360 : 0;
    animate();
}

QColor GLWidget::colorText() const
{
    return m_colorText;
}

void GLWidget::setColorText(const QColor &colorText)
{
    m_colorText = colorText;
}

QColor GLWidget::colorBackground() const
{
    return m_colorBackground;
}

void GLWidget::setColorBackground(const QColor &colorBackground)
{
    m_colorBackground = colorBackground;
}

void GLWidget::setFps(int fps)
{
    if (fps <= 0) return;
    m_targetFps = fps;
    setUpdatesEnabled(m_updatesEnabled);
}

QTime GLWidget::estimatedTime() const
{
    return m_estimatedTime;
}

void GLWidget::setEstimatedTime(const QTime &estimatedTime)
{
    m_estimatedTime = estimatedTime;
}

QTime GLWidget::spendTime() const
{
    return m_spendTime;
}

void GLWidget::setSpendTime(const QTime &spendTime)
{
    m_spendTime = spendTime;
}

void GLWidget::initializeDebugLogger()
{
    qDebug() << "[GLWidget] Initialize debug logger";
    QString glVersion = QString::fromUtf8((const char *)glGetString(GL_VERSION));
    QString glslVersion = QString::fromUtf8((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
    QString glVendor = QString::fromUtf8((const char *)glGetString(GL_VENDOR));
    QString glRenderer = QString::fromUtf8((const char *)glGetString(GL_RENDERER));
    qDebug() << "[GLWidget] OpenGL version: " << glVersion << "GLSL version: " << glslVersion << "Vendor: " << glVendor << "Renderer: " << glRenderer;
    QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
    logger->initialize();
}

void GLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    m_defaultShaderProgram = new QOpenGLShaderProgram();
    if (m_defaultShaderProgram) {
        if (!m_defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/base_vertex.glsl")) {
            qWarning() << "[GLWidget] Vertex shader compile error:" << m_defaultShaderProgram->log();
            m_error = true;
            return;
        }
        if (!m_defaultShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/base_fragment.glsl")) {
            qWarning() << "[GLWidget] Fragment shader compile error:" << m_defaultShaderProgram->log();
            m_error = true;
            return;
        }
        if (m_defaultShaderProgram->link() && m_defaultShaderProgram->isLinked()) {
            qDebug() << "[GLWidget] Base shader program created";
        } else {
            qWarning() << "[GLWidget] Base shader program link error:" << m_defaultShaderProgram->log();
            m_error = true;
            return;
        }
    }

    m_gcodeShaderProgram = new QOpenGLShaderProgram();
    if (m_gcodeShaderProgram) {
        m_gcodeShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/gcode_vertex.glsl");
        m_gcodeShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/gcode_fragment.glsl");
        if (m_gcodeShaderProgram->link() & m_gcodeShaderProgram->isLinked()) {
            qDebug() << "[GLWidget] GCode shader program created";
        } else {
            qWarning() << "[GLWidget] GCode shader program link error:" << m_gcodeShaderProgram->log();
            m_error = true;
            return;
        }
    }

    m_copyProgram = new QOpenGLShaderProgram();
    m_copyProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/2dcopy_vertex.glsl");
    m_copyProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/2dcopy_fragment.glsl");
    if (m_copyProgram->link() & m_copyProgram->isLinked()) {
        qDebug() << "[GLWidget] 2D Copy shader program created";
    } else {
        qWarning() << "[GLWidget] 2D Copy shader program link error:" << m_copyProgram->log();
        m_error = true;
        return;
    }
    m_copyProgram->setUniformValue("u_texture", 0);

    m_billboardShaderProgram = new QOpenGLShaderProgram();
    if (m_billboardShaderProgram) {
        if (!m_billboardShaderProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/billboard_vertex.glsl")) {
            qWarning() << "[GLWidget] Billboard vertex shader compile error:" << m_billboardShaderProgram->log();
            m_error = true;
            return;
        }
        if (!m_billboardShaderProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/billboard_fragment.glsl")) {
            qWarning() << "[GLWidget] Billboard fragment shader compile error:" << m_billboardShaderProgram->log();
            m_error = true;
            return;
        }
        if (m_billboardShaderProgram->link() && m_billboardShaderProgram->isLinked()) {
            qDebug() << "[GLWidget] Billboard shader program created";
            m_billboardShaderProgram->setUniformValue("u_billboardTexture", 1);
        } else {
            qWarning() << "[GLWidget] Billboard shader program link error:" << m_billboardShaderProgram->log();
            m_error = true;
            return;
        }
    }

    m_palette.initialize();

    initializeDebugLogger();
}

void GLWidget::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);
    updateProjection();
    emit resized();
}

void GLWidget::updateProjection()
{
    // Reset projection
    m_projectionMatrix.setToIdentity();

    double aspectRatio = (double)width() / height();

    // perspective / orthographic projection
    if (m_mode == ViewMode::Perspective) {
        m_projectionMatrix.perspective(m_fov, aspectRatio, m_near, m_far);
    } else {
        // Orthogonal and View2D use orthographic projection
        double orthoSize = m_zoomDistance;// * tan((m_fov * 0.0174533) / 2.0);
        m_projectionMatrix.ortho(-orthoSize * aspectRatio, orthoSize * aspectRatio, -orthoSize, orthoSize, -m_far/2.0, m_far/2.0);
    }
}

void GLWidget::updateView()
{
    m_viewMatrix.setToIdentity();

    // Convert angles to radians
    // m_xRot is Pitch (Elevation), m_yRot is Yaw (Azimuth)
    float pitch = qDegreesToRadians((float)m_xRot);
    float yaw = qDegreesToRadians((float)m_yRot);

    // Calculate eye position relative to lookAt using Spherical Coordinates (Z-up)
    // Pitch 0, Yaw 0 -> Front View (Looking from -Y towards +Y)
    // Pitch 90 -> Top View (Looking from +Z towards -Z)

    // We assume Front View is looking along Y axis (from negative to positive)
    // So at Yaw=0, Pitch=0, Eye should be at (0, -dist, 0)

    float cosPitch = cos(pitch);
    float sinPitch = sin(pitch);
    float cosYaw = cos(yaw);
    float sinYaw = sin(yaw);

    QVector3D offset(
        sinYaw * cosPitch,  // X
        -cosYaw * cosPitch, // Y (starts at -1 when yaw=0)
        sinPitch            // Z
    );

    m_eye = m_lookAt + offset * m_zoomDistance;

    // Calculate Up vector
    // The Up vector should be tangent to the sphere, pointing towards the north pole (Z+)
    // Derivative of position with respect to pitch gives the Up direction
    QVector3D up(
        -sinYaw * sinPitch,
        cosYaw * sinPitch,
        cosPitch
    );

    // Handle singularity at poles (Pitch +/- 90)
    // When looking straight down/up, the "Up" vector calculated above becomes (0,0,0)
    // In this case, we define Up as Y+ (rotated by Yaw) to maintain orientation
    if (qAbs(cosPitch) < 0.001f) {
        up = QVector3D(sinYaw, cosYaw, 0); // Y-axis rotated by Yaw
        if (pitch < 0) up = -up; // Invert for bottom view
    }

    up.normalize();

    m_cubeDrawer.updateEyePosition(m_eye, up);

    m_viewMatrix.lookAt(m_eye, m_lookAt, up);
    // Removed the extra rotate(-90) as we now calculate in Z-up space directly
}

void GLWidget::drawText(QPainter &painter, QPoint &pos, QString text, int lineHeight, Qt::AlignmentFlag align)
{
    int x = pos.x();
    if (align == Qt::AlignRight) {
        pos.setX(x - painter.fontMetrics().horizontalAdvance(text));
    }
    painter.drawText(pos, text);
    // revert X, advance Y
    pos.setX(x);
    pos.setY(pos.y() + lineHeight);
}

void GLWidget::drawTexts(QPainter &painter, QPoint &pos, QStringList texts, int lineHeight)
{
    foreach (QString text, texts) {
        drawText(painter, pos, text, lineHeight);
    }
}


#ifdef GLES
void GLWidget::paintGL() {
#else

void GLWidget::paintEvent(QPaintEvent *pe) {
    Q_UNUSED(pe)
#endif
    QPainter painter(this);

    if (m_error) {
        painter.setPen(Qt::red);
        painter.setFont(QFont("Arial", 16, QFont::Bold));
        painter.drawText(rect(), Qt::AlignCenter, "OpenGL error. See application log for details.");
        return;
    }

    // Segment counter
    int vertices = 0;

    painter.beginNativePainting();

    // Clear viewport
    glClearColor(m_colorBackground.redF(), m_colorBackground.greenF(), m_colorBackground.blueF(), 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Shader drawable points
    glEnable(GL_PROGRAM_POINT_SIZE);

    // Update settings
    if (m_antialiasing) {
        if (m_msaa) {
            glEnable(GL_MULTISAMPLE);
        } else {
            glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
            glEnable(GL_POINT_SMOOTH);
        }
    }
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    QOpenGLShaderProgram *currentProgram = nullptr;

    static float lightRotation = 0;
    QVector3D lightPos(100 * cos(lightRotation * M_PI / 180), 100 * sin(lightRotation * M_PI / 180), 40);
    lightRotation += 1;

    if (m_gcodeShaderProgram) {
        if (currentProgram && currentProgram != m_gcodeShaderProgram) {
            currentProgram->release();
        }
        currentProgram = m_gcodeShaderProgram;
        currentProgram->bind();
        currentProgram->setUniformValue("u_mvp_matrix", m_projectionMatrix * m_viewMatrix);
        currentProgram->setUniformValue("u_mv_matrix", m_viewMatrix);
        currentProgram->setUniformValue("u_light_position", lightPos);
        //
        currentProgram->setUniformValue("u_eye", m_eye);
        currentProgram->setUniformValue("u_near", (GLfloat) m_near);
        currentProgram->setUniformValue("u_far", (GLfloat) m_far);
    }

    if (m_defaultShaderProgram) {
        if (currentProgram && currentProgram != m_defaultShaderProgram) {
            currentProgram->release();
        }
        currentProgram = m_defaultShaderProgram;
        currentProgram->bind();
        currentProgram->setUniformValue("u_mvp_matrix", m_projectionMatrix * m_viewMatrix);
        currentProgram->setUniformValue("u_mv_matrix", m_viewMatrix);
        currentProgram->setUniformValue("u_light_position", lightPos);
    }

    if (m_billboardShaderProgram) {
        if (currentProgram && currentProgram != m_billboardShaderProgram) {
            currentProgram->release();
        }
        currentProgram = m_billboardShaderProgram;
        currentProgram->bind();
        currentProgram->setUniformValue("u_mvp_matrix", m_projectionMatrix * m_viewMatrix);
        currentProgram->release();
        currentProgram = m_defaultShaderProgram;
        if (currentProgram) currentProgram->bind();
    }

    foreach (ShaderDrawable *drawable, m_shaderDrawables) {
        if (!drawable->visible()) {
            continue;
        }
        QOpenGLShaderProgram *newProgram;
        switch (drawable->programType()) {
            case ShaderDrawable::ProgramType::GCode: {
                GcodeDrawer *gcodeDrawable = static_cast<GcodeDrawer*>(drawable);
                gcodeDrawable->setEyePos(m_eye);
                // gcodeDrawable->update();
                newProgram = m_gcodeShaderProgram;
                break;
            }
            case ShaderDrawable::ProgramType::Billboard:
                newProgram = m_billboardShaderProgram;
                break;
            default:
                newProgram = m_defaultShaderProgram;
                break;
        }
        if (currentProgram != newProgram) {
            if (currentProgram) {
                currentProgram->release();
            }
            currentProgram = newProgram;
            currentProgram->bind();
        }

        if (drawable->needsUpdateGeometry()) {
            drawable->updateGeometry(currentProgram, m_palette);
        }

        switch (drawable->programType()) {
        case ShaderDrawable::ProgramType::GCode: {
            m_palette.bind();
            drawable->draw(currentProgram);
            m_palette.release();
            break;
        }
        case ShaderDrawable::ProgramType::Billboard:
            // Set billboard-specific uniforms
            currentProgram->setUniformValue("u_mvp_matrix", m_projectionMatrix * m_viewMatrix);
            currentProgram->setUniformValue("u_billboardTexture", 1); // Texture unit 1
            m_palette.bind();
            drawable->draw(currentProgram);
            m_palette.release();
            break;
        case ShaderDrawable::ProgramType::Default:
            m_palette.bind();
            if (drawable->sort(m_viewMatrix)) {
                drawable->bindData(currentProgram);
            }
            drawable->draw(currentProgram);
            m_palette.release();
            break;
        }
        vertices += drawable->getVertexCount();
    }

    // Release current program before cube (cube uses its own shader)
    if (currentProgram != nullptr) {
        currentProgram->release();
    }

    // Don't show rotation cube in 2D mode
    if (m_rotationCube && m_mode != ViewMode::View2D) {
        glDisable(GL_DEPTH_TEST);
        m_cubeDrawer.draw(QRect(0, height() - 100, 100, 100), m_palette);

        // viewport was changed by cube drawer
        glViewport(0, 0, this->width(), this->height());
    }

    // Draw 2D
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);

    painter.endNativePainting();

    QPoint pos;

    QPen pen(m_colorText);
    painter.setPen(pen);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    const float scale = ThemeManager::instance().scale();
    const int lineHeight = 15 * scale;
    // text base point is at the bottom left corner, so we need to offset by font height
    const int fontHeight = painter.fontMetrics().height();

    // left side
    pos = QPoint(10, this->height() - 10 - (5 * lineHeight) + fontHeight);

    if (!qIsNaN(m_bottomSurfaceCursorPos.x())) {
        drawText(painter, pos, QString("Cursor: %1, %2").arg(m_bottomSurfaceCursorPos.x(), 0, 'f', 2).arg(m_bottomSurfaceCursorPos.y(), 0, 'f', 2), lineHeight);
    } else {
        drawText(painter, pos, "Cursor: ??", lineHeight);
    }
    drawText(painter, pos, QString("X: %1 ... %2").arg(m_xMin, 0, 'f', 3).arg(m_xMax, 0, 'f', 3), lineHeight);
    drawText(painter, pos, QString("Y: %1 ... %2").arg(m_yMin, 0, 'f', 3).arg(m_yMax, 0, 'f', 3), lineHeight);
    drawText(painter, pos, QString("Z: %1 ... %2").arg(m_zMin, 0, 'f', 3).arg(m_zMax, 0, 'f', 3), lineHeight);
    QString modeStr = m_mode == ViewMode::Perspective ? "p" : (m_mode == ViewMode::View2D ? "2d" : "o");
    drawText(painter, pos, QString("%1 / %2 / %3 / %4").arg(m_xSize, 0, 'f', 3).arg(m_ySize, 0, 'f', 3).arg(m_zSize, 0, 'f', 3).arg(modeStr), lineHeight);

    pos.setY(this->height() - 10 - (8 * lineHeight) + fontHeight);

    drawText(painter, pos, m_parserState, lineHeight);
    drawText(painter, pos, m_speedState, lineHeight);
    drawText(painter, pos, m_pinState, lineHeight);

    // right side
    pos = QPoint(this->width() - 10, this->height() - 10 - (4 * lineHeight) + fontHeight);

    drawText(painter, pos, m_spendTime.toString("hh:mm:ss") + " / " + m_estimatedTime.toString("hh:mm:ss"), lineHeight, Qt::AlignRight);
    drawText(painter, pos, m_bufferState, 15, Qt::AlignRight);
    drawText(painter, pos, QString(tr("Vertices: %1")).arg(vertices), lineHeight, Qt::AlignRight);
    drawText(painter, pos, QString("FPS: %1").arg(m_fps), lineHeight, Qt::AlignRight);

    m_frames++;
#ifdef GLES
    update();
#endif
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    // Block rotation cube clicks in 2D mode
    if (pos.x() < 100 && pos.y() < 100 && m_mode != ViewMode::View2D) {
        CubeClickableFace face = m_cubeDrawer.faceAtPos(pos);
        switch (face) {
            case CubeClickableFace::Front:
                setFrontView();
                break;
            case CubeClickableFace::Back:
                setBackView();
                break;
            case CubeClickableFace::Top:
                setTopView();
                break;
            case CubeClickableFace::Bottom:
                setBottomView();
                break;
            case CubeClickableFace::Left:
                setLeftView();
                break;
            case CubeClickableFace::Right:
                setRightView();
                break;
            case CubeClickableFace::None:
                break;
            default:
                break;
        }
    }

    m_lastPos = event->pos();
    m_xLastRot = m_xRot;
    m_yLastRot = m_yRot;
}

QPointF GLWidget::calcPositionOnXYPlane(QPoint mouseClickPosition)
{
    QVector2D normalizedPos(
        mouseClickPosition.x() / (width()  * 0.5f) - 1.0f,
        -(mouseClickPosition.y() / (height() * 0.5f) - 1.0f)
        );

    // Invert the matrices
    QMatrix4x4 invertedProjection = m_projectionMatrix.inverted();
    QMatrix4x4 invertedView = m_viewMatrix.inverted();

    // Convert 2D mouse position to 3D position with Z = -1 (near plane)
    QVector3D nearPlanePosition(normalizedPos, -1.0f);

    // Unproject the 3D position on the near plane to the world space
    QVector3D nearPlaneWorldPosition = invertedProjection.map(nearPlanePosition);
    nearPlaneWorldPosition = invertedView.map(nearPlaneWorldPosition);

    // Convert 2D mouse position to 3D position with Z = 1 (far plane)
    QVector3D farPlanePosition(normalizedPos, 1.0f);

    // Unproject the 3D position on the far plane to the world space
    QVector3D farPlaneWorldPosition = invertedProjection.map(farPlanePosition);
    farPlaneWorldPosition = invertedView.map(farPlaneWorldPosition);

    // Calculate the direction from the near plane to the far plane
    QVector3D direction = farPlaneWorldPosition - nearPlaneWorldPosition;

    // If the direction is parallel to the XY plane, then the click is not on the XY plane
    if (direction.z() == 0)
    {
        return QPointF(NAN, NAN);
    }

    // Calculate the intersection of the line with the XY plane (Z = 0)
    float t = -nearPlaneWorldPosition.z() / direction.z();
    QVector3D intersection = nearPlaneWorldPosition + direction * t;

    // Limit XY range
    if (abs(intersection.x()) > 3000 || abs(intersection.y()) > 3000) {
        return QPointF(NAN, NAN);
    }

    intersection.setX(round(intersection.x()));
    intersection.setY(round(intersection.y()));

    return intersection.toPointF();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    m_bottomSurfaceCursorPos = calcPositionOnXYPlane(pos);
    if (!qIsNaN(m_bottomSurfaceCursorPos.x())) {
        emit cursorPosChanged(m_bottomSurfaceCursorPos);
    }

    // Rotation: Middle button or Left button without Shift (but not in 2D mode)
    if (m_mode != ViewMode::View2D &&
        ((event->buttons() & Qt::MiddleButton && !(event->modifiers() & Qt::ShiftModifier))
        || (event->buttons() & Qt::LeftButton && !(event->modifiers() & Qt::ShiftModifier)))) {

        stopAnimation();

        m_yRot = normalizeAngle(m_yLastRot - (pos.x() - m_lastPos.x()) * 0.5);
        m_xRot = m_xLastRot + (pos.y() - m_lastPos.y()) * 0.5;

        if (m_xRot < -90) m_xRot = -90;
        if (m_xRot > 90) m_xRot = 90;

        updateView();
        emit rotationChanged();
    }

    // Panning: Right button, Shift+Middle, Shift+Left, or Left button in 2D mode
    if ((event->buttons() & Qt::MiddleButton && event->modifiers() & Qt::ShiftModifier)
        || event->buttons() & Qt::RightButton
        || (event->buttons() & Qt::LeftButton && (event->modifiers() & Qt::ShiftModifier))
        || (m_mode == ViewMode::View2D && event->buttons() & Qt::LeftButton))
    {
    #if NAV_MODE == 1
        // Get world to clip
        QMatrix4x4 mvp(m_projectionMatrix * m_viewMatrix);
        // Get clip to world
        QMatrix4x4 mvpi(mvp.inverted());

        QVector4D centerVector(mvp * QVector4D(m_lookAt.x(),m_lookAt.y(),m_lookAt.z(), 1.0));

        // Get last mouse XY in clip
        QVector4D lastMouseInWorld(
            (m_lastPos.x() / (double)width()) * 2.0 - 1.0,
            -((m_lastPos.y() / (double)height()) * 2.0 - 1.0),
            0,
            1.0
        );
        // Project last mouse pos to world
        lastMouseInWorld = mvpi * lastMouseInWorld * centerVector.w();

        // Get current mouse XY in clip
        QVector4D currentMouseInWorld(
            (pos.x() / (double)width()) * 2.0 - 1.0,
            -((pos.y() / (double)height()) * 2.0 - 1.0),
            0,
            1.0
        );
        // Project current mouse pos to world
        currentMouseInWorld = mvpi * currentMouseInWorld * centerVector.w();

        //currentMouseInWorld /= currentMouseInWorld.w();

        // Get difference
        QVector4D difference = currentMouseInWorld - lastMouseInWorld;

        // Subtract difference from center point
        m_lookAt -= QVector3D(difference.x(), difference.y(), difference.z());
    #endif
    #if NAV_MODE == 2
        // Move in camera local space ( screen space )
        // Calculate "right" and "up" vectors relative to the camera
        QVector3D direction = (m_lookAt - m_eye).normalized();

        // Use Z-up for world up vector
        QVector3D worldUp(0, 0, 1);

        // Calculate camera Right vector
        QVector3D right = QVector3D::crossProduct(direction, worldUp).normalized();

        // Handle singularity when looking straight down/up
        if (right.lengthSquared() < 0.001f) {
             // If looking down/up, Right is X-axis rotated by Yaw
             float yaw = qDegreesToRadians((float)m_yRot);
             right = QVector3D(cos(yaw), -sin(yaw), 0);
        }

        // Calculate camera Up vector (perpendicular to direction and right)
        QVector3D up = QVector3D::crossProduct(right, direction).normalized();

        // Mouse movement in pixels
        double dx = pos.x() - m_lastPos.x();
        double dy = pos.y() - m_lastPos.y();

        // Scale movement (you can adjust the factor)
        double moveScale = m_zoomDistance * 0.006;

        QVector3D move = -right * dx * moveScale + up * dy * moveScale;
        m_lookAt += move;
        m_eye += move;
    #endif

        m_lastPos = pos;

        updateView();
    }

    if (pos.x() < 200 && pos.y() < 200) {
        CubeClickableFace face = m_cubeDrawer.mouseMoveEvent(event);
        if (face != CubeClickableFace::None) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF cursorPos = calcPositionOnXYPlane(event->pos());
        if (!qIsNaN(cursorPos.x()) && !qIsNaN(cursorPos.y())) {
            emit goToCursor(cursorPos);
        }
    }
}

#ifndef USE_GLWINDOW
void GLWidget::leaveEvent(QEvent *event)
{
    QOpenGLWidget::leaveEvent(event);
    m_cubeDrawer.leaveEvent(event);
    emit left();
}
#endif

void GLWidget::wheelEvent(QWheelEvent *we)
{
    int delta = we->angleDelta().y();
#if NAV_MODE == 1
    if (m_zoomDistance > MIN_ZOOM && delta < 0) {
        m_zoomDistance /= ZOOMSTEP;
    } else if (delta > 0) {
        m_zoomDistance *= ZOOMSTEP;
    }

    if (m_mode != ViewMode::Perspective) {
        updateProjection();
    } else {
        updateView();
    }
#endif
#if NAV_MODE == 2
    double zoomStep = (delta > 0) ? ZOOMSTEP : 1.0 / ZOOMSTEP;
    if (m_mode == ViewMode::Perspective) {
        // Move the camera and lookAt point along the view direction
        QVector3D viewDir = (m_lookAt - m_eye).normalized();
        double moveDist = m_zoomDistance * (zoomStep - 1.0);
        m_eye += viewDir * moveDist;
        m_lookAt += viewDir * moveDist;
        m_zoomDistance = qBound(MIN_ZOOM, (m_eye - m_lookAt).length(), MAX_ZOOM);
    } else {
        // Ortho and View2D: scale only
        m_zoomDistance *= zoomStep;
        m_zoomDistance = qBound(MIN_ZOOM, m_zoomDistance, MAX_ZOOM);
    }
    emitZoomChanged();
    updateProjection();
    updateView();
#endif
}

void GLWidget::timerEvent(QTimerEvent *te)
{
    if (te->timerId() == m_timerPaint.timerId()) {
        if (m_animateView) {
            onAnimation();
        }
#ifndef GLES
        update();
#endif
    } else {
#ifdef GLES
#ifdef USE_GLWINDOW
        QOpenGLWindow::timerEvent(te);
#else
        QOpenGLWidget::timerEvent(te);
#endif
#else
        QGLWidget::timerEvent(te);
#endif
    }
}

double GLWidget::normalizeAngle(double angle)
{
    while (angle < 0) angle += 360;
    while (angle > 360) angle -= 360;

    return angle;
}

#ifndef USE_GLWINDOW
void GLWidget::enterEvent(QEnterEvent *event)
{
    QOpenGLWidget::enterEvent(event);
    emit entered();
}
#endif
