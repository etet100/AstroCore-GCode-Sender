//#define sNan qQNaN();

#include "shaderdrawable.h"

#ifdef GLES
//#include <GLES/gl.h>
#endif

ShaderDrawable::ShaderDrawable()
{
    m_needsUpdateGeometry = true;
    m_visible = true;
    m_lineWidth = 1.0;
    m_pointSize = 1.0;
    //m_texture = NULL;
}

ShaderDrawable::~ShaderDrawable()
{
    //if (!m_vao.isCreated()) m_vao.destroy();
    if (!m_vbo.isCreated()) m_vbo.destroy();
}

void ShaderDrawable::init()
{
    // Init openGL functions
    initializeOpenGLFunctions();

    // Create buffers
    //m_vao.create();
    m_vbo.create();
}

void ShaderDrawable::update()
{
    m_needsUpdateGeometry = true;
}

void ShaderDrawable::bindAttributes(QOpenGLShaderProgram *&shaderProgram)
{
    quintptr offset = 0;
    int pos;

    // Tell OpenGL programmable pipeline how to locate vertex position data
    pos = shaderProgram->attributeLocation("a_position");
    assert(pos >= 0);
    shaderProgram->enableAttributeArray(pos);
    shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 3, sizeof(VertexData));

    // Offset for color
    offset = sizeof(QVector3D);

    // Tell OpenGL programmable pipeline how to locate vertex color data
    pos = shaderProgram->attributeLocation("a_color");
    assert(pos >= 0);
    shaderProgram->enableAttributeArray(pos);
    shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 1, sizeof(VertexData));

    // Offset for line start point
    offset += sizeof(GLfloat);

    // Tell OpenGL programmable pipeline how to locate vertex line start point
    pos = shaderProgram->attributeLocation("a_start");
    if (pos > 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 3, sizeof(VertexData));
    }

    offset += sizeof(QVector3D);

    pos = shaderProgram->attributeLocation("a_cumSegPosition");
    if (pos > 0) {
        shaderProgram->enableAttributeArray(pos);
        shaderProgram->setAttributeBuffer(pos, GL_FLOAT, offset, 1, sizeof(VertexData));
    }
}

void ShaderDrawable::updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette)
{
    // Init in context
    if (!m_vbo.isCreated())
        init();

    // if (m_vao.isCreated()) {
    //     //m_vao.bind();
    // }

    // Prepare vbo
    m_vbo.bind();

    // Update vertex buffer
    if (updateData(palette)) {
        // Fill vertices buffer
        QVector<VertexData> vertexData(m_triangles);
        vertexData += m_lines;
        vertexData += m_points;
        m_vbo.allocate(vertexData.constData(),
                       vertexData.count() * sizeof(VertexData));
    } else {
        m_vbo.release();
        // if (m_vao.isCreated())
        //     m_vao.release();
        m_needsUpdateGeometry = false;

        return;
    }

    // if (m_vao.isCreated()) {
    //     bindAttributes(shaderProgram);
    //     m_vao.release();
    // }

    m_vbo.release();

    m_needsUpdateGeometry = false;
}

bool ShaderDrawable::updateData(GLPalette &palette)
{
    // Test data
    m_lines = QVector<VertexData>()
        << VertexData(QVector3D(0, 0, 0), palette.color(1, 0, 0), QVector3D(sNan, 0, 0))
        << VertexData(QVector3D(10, 0, 0), palette.color(1, 1, 0), QVector3D(sNan, 0, 0))
        << VertexData(QVector3D(0, 0, 0), palette.color(0, 1, 0), QVector3D(sNan, 0, 0))
        << VertexData(QVector3D(0, 10, 0), palette.color(0, 1, 0), QVector3D(sNan, 0, 0))
        << VertexData(QVector3D(0, 0, 0), palette.color(0, 0, 1), QVector3D(sNan, 0, 0))
        << VertexData(QVector3D(0, 0, 10), palette.color(0, 0, 1), QVector3D(sNan, 0, 0));

    return true;
}

bool ShaderDrawable::needsUpdateGeometry() const
{
    return m_needsUpdateGeometry;
}

void ShaderDrawable::draw(QOpenGLShaderProgram *shaderProgram)
{
    if (!m_visible) return;

    // if (m_vao.isCreated()) {
    //     m_vao.bind();
    // } else {
        m_vbo.bind();        
        bindAttributes(shaderProgram);
    // }

    // setAttributeBuffer must used every time, because it is not stored in VAO??
    //shaderProgram->setAttributeValue("a_alpha", m_globalAlpha);

    // if (!m_triangles.isEmpty()) {
    //     // if (m_texture) {
    //     //     m_texture->bind();
    //     //     shaderProgram->setUniformValue("texture", 0);
    //     // }
    //     glDrawArrays(GL_TRIANGLES, 0, m_triangles.count());
    // }

    if (!m_lines.isEmpty()) {
        glLineWidth(m_lineWidth);
        glDrawArrays(GL_LINES, m_triangles.count(), m_lines.count());
    }

    // if (!m_points.isEmpty()) {
    //     glDrawArrays(GL_POINTS, m_triangles.count() + m_lines.count(), m_points.count());
    // }

    //if (m_vao.isCreated()) m_vao.release(); else
    m_vbo.release();
}

QVector3D ShaderDrawable::getSizes()
{
    return QVector3D(0, 0, 0);
}

QVector3D ShaderDrawable::getMinimumExtremes()
{
    return QVector3D(0, 0, 0);
}

QVector3D ShaderDrawable::getMaximumExtremes()
{
    return QVector3D(0, 0, 0);
}

int ShaderDrawable::getVertexCount()
{
    return m_lines.count();// + m_points.count() + m_triangles.count();
}

double ShaderDrawable::lineWidth() const
{
    return m_lineWidth;
}

void ShaderDrawable::setLineWidth(double lineWidth)
{
    m_lineWidth = lineWidth;
}

bool ShaderDrawable::visible() const
{
    return m_visible;
}

void ShaderDrawable::setVisible(bool visible)
{
    m_visible = visible;
}

double ShaderDrawable::pointSize() const
{
    return m_pointSize;
}

void ShaderDrawable::setPointSize(double pointSize)
{
    m_pointSize = pointSize;
}
