#include "glframebuffer.h"
#include <QFile>
#include <QDebug>

GLFramebuffer::GLFramebuffer(int width, int height) : QOpenGLFramebufferObject(width, height),
    m_colorTexture(QOpenGLTexture::Target2D), m_depthTexture(QOpenGLTexture::Target2D),
    m_glfuncs(new QOpenGLFunctions_3_0())
{
    m_colorTexture.create();
    m_colorTexture.bind();
    m_colorTexture.setSize(width, height);
    m_colorTexture.setFormat(QOpenGLTexture::RGBA8_UNorm);
    m_colorTexture.allocateStorage(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8);
    m_colorTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
    m_colorTexture.setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_colorTexture.release();

    m_depthTexture.create();
    m_depthTexture.bind();
    m_depthTexture.setSize(width, height);
    m_depthTexture.setFormat(QOpenGLTexture::D32F);
    m_depthTexture.allocateStorage(QOpenGLTexture::Depth, QOpenGLTexture::Float32);
    m_depthTexture.setWrapMode(QOpenGLTexture::ClampToEdge);
    m_depthTexture.setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    m_depthTexture.release();
}

GLFramebuffer::~GLFramebuffer()
{
    m_colorTexture.destroy();
    m_depthTexture.destroy();
    delete m_glfuncs;
}

bool GLFramebuffer::bind()
{
    if (!m_initialized) {
        m_glfuncs->initializeOpenGLFunctions();
        m_initialized = true;
    }

    bool ok = QOpenGLFramebufferObject::bind();
    if (!ok) {
        return false;
    }

    m_glfuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture.textureId(), 0);
    m_glfuncs->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture.textureId(), 0);

    return true;
}

bool GLFramebuffer::release()
{
    return QOpenGLFramebufferObject::release();
}

void GLFramebuffer::dumpColorTexture(QString filename)
{
    GLuint *pixels = new GLuint[width() * height()];
    m_glfuncs->glBindTexture(GL_TEXTURE_2D, m_colorTexture.textureId());
    m_glfuncs->glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    QImage img((uchar *)pixels, width(), height(), QImage::Format_RGBA8888);
    img.save(QString("%1.png").arg(filename));

    delete[] pixels;
}

void GLFramebuffer::dumpDepthTexture(QString filename)
{
    GLfloat *pixels = new GLfloat[width() * height()];
    m_glfuncs->glBindTexture(GL_TEXTURE_2D, m_depthTexture.textureId());
    m_glfuncs->glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

    uint16_t *pixels16 = new uint16_t[width() * height() + (2 * height())];

    int i = 0;
    int j = 0;
    GLfloat min = 1000.0;
    GLfloat max = -1000.0;
    // for (int y = 0; y < height(); y++) {
    //     for (int x = 0; x < width(); x++) {
    //         min = std::min(min, pixels[j]);
    //         if (pixels[j] < 0.999f) {
    //             max = std::max(max, pixels[j]);
    //         }
    //         j++;
    //     }
    // }

    min = 0;
    max = 1.0;


    qDebug() << "Min: " << min << "Max: " << max;

    GLfloat mul =
        (max > min) ? 1.0 / (max - min) : 1.0;

    i = 0;
    j = 0;
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            if (mul == 1.0) {
                pixels16[i++] = pixels[j++] * 60000;
            } else {
                pixels16[i++] = std::min((int)((pixels[j++] - min) * mul * 60000), 60000);
            }
        }
        i += width() % 2;
    }

    QImage img((uchar *)pixels16, width(), height(), QImage::Format_Grayscale16);
    img.save(QString("%1.tif").arg(filename));

    delete[] pixels;
    delete[] pixels16;
}

QString GLFramebuffer::dumpDepthMinMax()
{
    GLfloat *pixels = new GLfloat[width() * height()];
    m_glfuncs->glBindTexture(GL_TEXTURE_2D, m_depthTexture.textureId());
    m_glfuncs->glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

    int j = 0;
    GLfloat min = 10.0;
    GLfloat max = -10.0;
    for (int y = 0; y < height(); y++) {
        for (int x = 0; x < width(); x++) {
            min = std::min(min, pixels[j]);
            if (pixels[j] < 0.9999f) {
                max = std::max(max, pixels[j]);
            }
            j++;
        }
    }

    delete[] pixels;

    return QString("Min: %1, Max: %2").arg(min).arg(max);
}
