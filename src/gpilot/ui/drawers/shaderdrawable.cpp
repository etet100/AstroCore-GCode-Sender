//#define sNan qQNaN();

#include "shaderdrawable.h"

#ifdef GLES
// #include <GLES/gl.h>
#endif

ShaderDrawable::ShaderDrawable()
{
}

ShaderDrawable::~ShaderDrawable()
{
    if (m_vao.isCreated()) m_vao.destroy();
    if (m_vbo.isCreated()) m_vbo.destroy();
}

void ShaderDrawable::init()
{
    // Init openGL functions
    initializeOpenGLFunctions();
    if (!m_vao.isCreated()) m_vao.create();
    if (!m_vbo.isCreated()) m_vbo.create();
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
    pos = shaderProgram->attributeLocation("a_normal");
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

void ShaderDrawable::updateGeometry(QOpenGLShaderProgram* shaderProgram, GLPalette& palette)
{
    // Init in context, it has to be done before updateData is called
    if (!m_vbo.isCreated() || !m_vao.isCreated()) {
        init();
    }

    m_vao.bind();
    m_vbo.bind();

    // Update vertex buffer, if async data update is used, updateData will return false and
    // updateVerticesBuffers will be called later
    if (updateData(palette)) {
        // Fill vertices buffer
        QVector<VertexData> vertexData(m_triangles);
        vertexData += m_lines;
        vertexData += m_points;
        m_vbo.allocate(vertexData.constData(),
                       vertexData.count() * sizeof(VertexData));
    }
    bindAttributes(shaderProgram);

    m_vbo.release();
    m_vao.release();
    m_needsUpdateGeometry = false;
}

void ShaderDrawable::updateVerticesBuffers()
{
    // Init in context, it has to be done before updateData is called
    if (!m_vbo.isCreated() || !m_vao.isCreated()) {
        init();
    }

    m_vao.bind();
    m_vbo.bind();

    // Fill vertices buffer
    QVector<VertexData> vertexData(m_triangles);
    vertexData += m_lines;
    vertexData += m_points;
    m_vbo.allocate(vertexData.constData(),
                   vertexData.count() * sizeof(VertexData));

    m_vbo.release();
    m_vao.release();
    m_needsUpdateGeometry = false;
}

void ShaderDrawable::bindData(QOpenGLShaderProgram *shaderProgram)
{
    // Init in context
    if (!m_vbo.isCreated() || !m_vao.isCreated()) {
        init();
    }

    m_vao.bind();
    m_vbo.bind();

    // Fill vertices buffer
    QVector<VertexData> vertexData(m_triangles);
    vertexData += m_lines;
    vertexData += m_points;
    m_vbo.allocate(vertexData.constData(),
                   vertexData.count() * sizeof(VertexData));
    bindAttributes(shaderProgram);

    m_vbo.release();
    m_vao.release();
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

bool ShaderDrawable::sort(QMatrix4x4 viewMatrix)
{
    return false;
}

bool ShaderDrawable::needsUpdateGeometry() const
{
    return m_needsUpdateGeometry;
}

void ShaderDrawable::setTranslation(const QVector3D &translation)
{
    m_translation = translation;
    rebuildModelMatrix();
}

void ShaderDrawable::setRotation(float angle, const QVector3D &axis)
{
    m_rotation = QQuaternion::fromAxisAndAngle(axis, angle);
    rebuildModelMatrix();
}

void ShaderDrawable::setRotation(float x, float y, float z)
{
    m_rotation = QQuaternion::fromEulerAngles(x, y, z);
    rebuildModelMatrix();
}

void ShaderDrawable::setOrigin(const QVector3D &origin)
{
    m_origin = origin;
    rebuildModelMatrix();
}

const QMatrix4x4& ShaderDrawable::modelMatrix() const
{
    return m_modelMatrix;
}

void ShaderDrawable::rebuildModelMatrix()
{
    m_modelMatrix.setToIdentity();
    // T(translation + origin) * R * T(-origin)
    // Result: rotates around origin, then translates
    m_modelMatrix.translate(m_translation + m_origin);
    m_modelMatrix.rotate(m_rotation);
    m_modelMatrix.translate(-m_origin);
}

void ShaderDrawable::draw(QOpenGLShaderProgram *shaderProgram)
{
    if (!m_visible) {
        return;
    }

    shaderProgram->setUniformValue("u_model_matrix", m_modelMatrix);
    if (m_vao.isCreated()) {
        m_vao.bind();
    } else {
        m_vbo.bind();
        bindAttributes(shaderProgram);
    }

    if (m_depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    // Temporary solution for z-fighting in heightmap visualization
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    if (!m_triangles.isEmpty()) {
        shaderProgram->setUniformValue("u_flat_shading", 0);
        glDrawArrays(GL_TRIANGLES, 0, m_triangles.count());
    }
    shaderProgram->setUniformValue("u_flat_shading", (GLfloat)m_flatShading);
    glDisable(GL_POLYGON_OFFSET_FILL);
    if (!m_lines.isEmpty()) {
        glLineWidth(m_lineWidth);
        glDrawArrays(GL_LINES, m_triangles.count(), m_lines.count());
    }
    if (!m_points.isEmpty()) {
        glEnable(GL_PROGRAM_POINT_SIZE);
        shaderProgram->setUniformValue("u_point_size", (GLfloat)m_pointSize);
        glDrawArrays(GL_POINTS, m_triangles.count() + m_lines.count(), m_points.count());
    }

    if (m_vao.isCreated()) {
        m_vao.release();
    } else {
        m_vbo.release();
    }
}

QVector3D ShaderDrawable::sizes()
{
    return QVector3D(0, 0, 0);
}

QVector3D ShaderDrawable::minimumExtremes()
{
    return QVector3D(0, 0, 0);
}

QVector3D ShaderDrawable::maximumExtremes()
{
    return QVector3D(0, 0, 0);
}

int ShaderDrawable::getVertexCount()
{
    return m_lines.count() + m_points.count() + m_triangles.count();
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

void ShaderDrawable::toggleVisible()
{
    m_visible = !m_visible;
}

void ShaderDrawable::setPointSize(double pointSize)
{
    m_pointSize = pointSize;
}
