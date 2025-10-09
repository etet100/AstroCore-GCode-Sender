#include "glcontainer.h"
#include <QLayout>
#include <QTime>

#ifdef USE_GLWINDOW

GLContainer::GLContainer(QWidget *parent)
    : QWidget(parent)
{
    m_glWidget = new GLWidget();

    QWidget *container = createWindowContainer(m_glWidget);

    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
    layout->addWidget(container);
    layout->setContentsMargins(0, 0, 0, 0);
}

void GLContainer::addDrawable(ShaderDrawable *drawable)
{
    m_glWidget->addDrawable(drawable);
}

GLContainer &GLContainer::operator<<(ShaderDrawable *drawable)
{
    m_glWidget->addDrawable(drawable);

    return *this;
}

void GLContainer::fitDrawable(ShaderDrawable *drawable)
{
    m_glWidget->fitDrawable(drawable);
}

void GLContainer::updateExtremes(ShaderDrawable *drawable)
{
    m_glWidget->updateExtremes(drawable);
}

bool GLContainer::antialiasing() const
{
    return m_glWidget->antialiasing();
}

void GLContainer::setAntialiasing(bool antialiasing)
{
    m_glWidget->setAntialiasing(antialiasing);
}

QString GLContainer::pinState() const
{
    return m_glWidget->pinState();
}

void GLContainer::setPinState(const QString &pinState)
{
    m_glWidget->setPinState(pinState);
}

void GLContainer::updateDrawer(ShaderDrawable *drawer)
{
    m_glWidget->updateDrawer(drawer);
}

void GLContainer::showEvent(QShowEvent *event)
{
    m_glWidget->show();

    QWidget::showEvent(event);
}

void GLContainer::resizeEvent(QResizeEvent *event)
{
    m_glWidget->setWidth(event->size().width());
    m_glWidget->setHeight(event->size().height());

    QWidget::resizeEvent(event);
}

QString GLContainer::speedState() const
{
    return m_glWidget->speedState();
}

void GLContainer::setSpeedState(const QString &additionalStatus)
{
    m_glWidget->setSpeedState(additionalStatus);
}

bool GLContainer::vsync() const
{
    return m_glWidget->vsync();
}

void GLContainer::setVsync(bool vsync)
{
    m_glWidget->setVsync(vsync);
}

bool GLContainer::msaa() const
{
    return m_glWidget->msaa();
}

void GLContainer::setMsaa(bool msaa)
{
    m_glWidget->setMsaa(msaa);
}

bool GLContainer::updatesEnabled() const
{
    return m_glWidget->updatesEnabled();
}

void GLContainer::setUpdatesEnabled(bool updatesEnabled)
{
    m_glWidget->setUpdatesEnabled(updatesEnabled);
}

bool GLContainer::zBuffer() const
{
    return m_glWidget->zBuffer();
}

void GLContainer::setZBuffer(bool zBuffer)
{
    m_glWidget->setZBuffer(zBuffer);
}

double GLContainer::fov()
{
    return m_glWidget->fov();
}

void GLContainer::setFov(double fov)
{
    m_glWidget->setFov(fov);
}

double GLContainer::nearPlane()
{
    return m_glWidget->nearPlane();
}

void GLContainer::setNearPlane(double plane)
{
    m_glWidget->setNearPlane(plane);
}

double GLContainer::farPlane()
{
    return m_glWidget->farPlane();
}

void GLContainer::setFarPlane(double plane)
{
    m_glWidget->setFarPlane(plane);
}

QString GLContainer::bufferState() const
{
    return m_glWidget->bufferState();
}

void GLContainer::setBufferState(const QString &bufferState)
{
    m_glWidget->setBufferState(bufferState);
}

QString GLContainer::parserState() const
{
    return m_glWidget->parserState();
}

void GLContainer::setParserState(const QString &parserState)
{
    m_glWidget->setParserState(parserState);
}

double GLContainer::lineWidth() const
{
    return m_glWidget->lineWidth();
}

void GLContainer::setLineWidth(double lineWidth)
{
    m_glWidget->setLineWidth(lineWidth);
}

void GLContainer::setTopView()
{
    m_glWidget->setTopView();
}

void GLContainer::setBottomView()
{
    m_glWidget->setBottomView();
}

void GLContainer::setFrontView()
{
    m_glWidget->setFrontView();
}

void GLContainer::setBackView()
{
    m_glWidget->setBackView();
}

void GLContainer::setLeftView()
{
    m_glWidget->setLeftView();
}

void GLContainer::setRightView()
{
    m_glWidget->setRightView();
}

int GLContainer::fps()
{
    return m_glWidget->fps();
}

void GLContainer::toggleProjectionType()
{
    m_glWidget->toggleProjectionType();
}

void GLContainer::toggleRotationCube()
{
    m_glWidget->toggleRotationCube();
}

void GLContainer::setIsometricView()
{
    m_glWidget->setIsometricView();
}

QColor GLContainer::colorText() const
{
    return m_glWidget->colorText();
}

void GLContainer::setColorText(const QColor &colorText)
{
    m_glWidget->setColorText(colorText);
}

QColor GLContainer::colorBackground() const
{
    return m_glWidget->colorBackground();
}

void GLContainer::setColorBackground(const QColor &colorBackground)
{
    m_glWidget->setColorBackground(colorBackground);
}

void GLContainer::setFps(int fps)
{
    m_glWidget->setFps(fps);
}

QTime GLContainer::estimatedTime() const
{
    return m_glWidget->estimatedTime();
}

void GLContainer::setEstimatedTime(const QTime &estimatedTime)
{
    m_glWidget->setEstimatedTime(estimatedTime);
}

QTime GLContainer::spendTime() const
{
    return m_glWidget->spendTime();
}

void GLContainer::setSpendTime(const QTime &spendTime)
{
    m_glWidget->setSpendTime(spendTime);
}

#endif
