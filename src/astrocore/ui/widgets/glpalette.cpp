#include "glpalette.h"

// has to be changed in vertex shaders too
#define MAX_COLORS 100

GLPalette::GLPalette() : m_colors(), m_indexes() {
    m_texture = nullptr;
}

void GLPalette::initialize()
{
    m_texture = nullptr;
    generateTexture();
}

void GLPalette::bind(GLuint unit)
{
    if (m_updated) {
        generateTexture();
    }

    if (unit == -1) {
        m_texture->bind();
    } else {
        m_texture->bind(unit);
    }
}

void GLPalette::release()
{
    m_texture->release();
}

GLuint GLPalette::add(float r, float g, float b, float a)
{
    assert(m_colors.count() < MAX_COLORS);
    assert(r <= 1.0 && g <= 1.0 && b <= 1.0);

    QString index = QString("%1_%2_%3_%4").arg(r).arg(g).arg(b).arg(a);

    m_colors << GLColor(r, g, b, a);
    m_indexes[index] = m_colors.count() - 1;
    m_updated = true;

    return m_colors.count() - 1;
}

GLuint GLPalette::add(const GLColor &color)
{
    // Alpha must be kept — the lookup key in color() includes it.
    return add(color.x(), color.y(), color.z(), color.w());
}

int GLPalette::count()
{
    return m_colors.count();
}

QString GLPalette::colorAsHex(int index)
{
    GLColor color = m_colors[index];

    return QString("#%1%2%3").arg((int)(color.x() * 255), 2, 16, QChar('0'))
                            .arg((int)(color.y() * 255), 2, 16, QChar('0'))
                            .arg((int)(color.z() * 255), 2, 16, QChar('0'));
}

GLPalette &GLPalette::operator <<(const GLColor &color)
{
    add(color.x(), color.y(), color.z(), color.w());

    return *this;
}

void GLPalette::generateTexture()
{
    if (m_texture == nullptr) {
        m_texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
        m_texture->create();
        m_texture->setFormat(QOpenGLTexture::RGBA32F);
        m_texture->setSize(MAX_COLORS, 1);
        m_texture->allocateStorage();
    }

    assert(m_colors.size() <= MAX_COLORS);

    GLfloat colorPalette[MAX_COLORS][4];

    for (int i = 0; i < m_colors.count(); i++) {
        GLColor color = m_colors[i];
        colorPalette[i][0] = color.x();
        colorPalette[i][1] = color.y();
        colorPalette[i][2] = color.z();
        colorPalette[i][3] = color.w();
    }
    for (int i = m_colors.count(); i < MAX_COLORS; i++) {
        colorPalette[i][0] = 1.0;
        colorPalette[i][1] = 1.0;
        colorPalette[i][2] = 1.0;
        colorPalette[i][3] = 1.0;
    }

    m_texture->setData(QOpenGLTexture::RGBA, QOpenGLTexture::Float32, colorPalette);

    m_updated = false;
}

