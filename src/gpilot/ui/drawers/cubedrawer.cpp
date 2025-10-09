#include "cubedrawer.h"

#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include "cube.h"
#include "utils/utils.h"
#include "shaderdrawable.h"

#define DISTANCE 50.0
#define SIZE 100.0
#define MULTISAMPLE 2

CubeDrawer::CubeDrawer() : m_eye({0, 0, 1})
{
    m_width = SIZE * MULTISAMPLE;
    m_height = SIZE * MULTISAMPLE;
    m_triangles = cube;

    setProjection();
}

void CubeDrawer::drawBackground()
{
    glClearColor(0.2, 0.2, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void CubeDrawer::drawToBuffer(GLPalette &palette)
{
    m_fbo->bind();
    glViewport(0, 0, this->m_width, this->m_height);
    glEnable(GL_BLEND);
    drawBackground();
    drawCube(palette);
    m_fbo->release();
}

void CubeDrawer::copyToScreen(QRect dest)
{
    m_copyProgram->bind();

    QOpenGLBuffer copyVbo;
    copyVbo.create();
    copyVbo.bind();

    quintptr offset = 0;

    int vertexLocation = m_copyProgram->attributeLocation("a_position");
    m_copyProgram->enableAttributeArray(vertexLocation);
    m_copyProgram->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 2, 2 * 4 * 2);

    offset += sizeof(QVector2D);

    int textureLocation = m_copyProgram->attributeLocation("a_texcoord");
    m_copyProgram->enableAttributeArray(textureLocation);
    m_copyProgram->setAttributeBuffer(textureLocation, GL_FLOAT, offset, 2, 2 * 4 * 2);

    m_copyProgram->setUniformValue("u_mode", 3);
    m_copyProgram->setUniformValue("u_resolution", QVector2D(SIZE * MULTISAMPLE, SIZE * MULTISAMPLE));
    m_copyProgram->setUniformValue("u_texture", 0);

    QVector<_2DTexturedVertexData> copyTriangles;
    // copyTriangles << _2DTexturedVertexData(QVector2D(-1.0, -1.0), QVector2D(0, 0))
    //               << _2DTexturedVertexData(QVector2D(1.0, -1.0), QVector2D(1, 0))
    //               << _2DTexturedVertexData(QVector2D(-1.0, 1.0), QVector2D(0, 1))
    //               << _2DTexturedVertexData(QVector2D(-1.0, 1.0), QVector2D(0, 1))
    //               << _2DTexturedVertexData(QVector2D(1.0, -1.0), QVector2D(1, 0))
    //               << _2DTexturedVertexData(QVector2D(1.0, 1.0), QVector2D(1, 1));
    // use single triangle to cover the whole screen
    copyTriangles << _2DTexturedVertexData(QVector2D(-1.0, -1.0), QVector2D(0, 0))
                << _2DTexturedVertexData(QVector2D(-1.0, 3.0), QVector2D(0, 2))
                << _2DTexturedVertexData(QVector2D(3.0, -1.0), QVector2D(2, 0));

    copyVbo.allocate(copyTriangles.constData(), copyTriangles.count() * sizeof(_2DTexturedVertexData));

    glViewport(dest.x(), dest.y(), dest.width(), dest.height());
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_fbo->texture());

    glDrawArrays(GL_TRIANGLES, 0, copyTriangles.count());

    copyVbo.release();
    copyVbo.destroy();

    m_copyProgram->release();
}

// dest is normalized to -1, 1
void CubeDrawer::draw(QRect dest, GLPalette &palette)
{
    if (!m_vbo.isCreated()) {
        init();
    }
    drawToBuffer(palette);
    copyToScreen(dest);
}

void CubeDrawer::setProjection()
{
    m_projectionMatrix.setToIdentity();
    m_projectionMatrix.ortho(-10, 10, -10, 10, 1, 70);
}

void CubeDrawer::updateView()
{
    m_viewMatrix.setToIdentity();

    // use the eye position of main scene to rotate cube
    QVector3D normalized = m_eye.normalized();
    QVector3D eye = normalized * DISTANCE;

    m_viewMatrix.lookAt(eye, QVector3D(0, 0, 0), m_up);
    m_viewMatrix.rotate(90, 0.0, 1.0, 0.0);
    m_viewMatrix.rotate(-90, 1.0, 0.0, 0.0);
}

void CubeDrawer::updateEyePosition(QVector3D eye, QVector3D up)
{
    m_eye = eye;
    m_up = up;
    // m_needsUpdateGeometry = true;

    updateView();

    QMatrix4x4 mvp = m_projectionMatrix * m_viewMatrix;
    m_points2d.clear();
    for (auto clickable : clickables) {
        for (int i = 0; i < 6; i++) {
            QPointF mapped = (mvp.map(cube[clickable[i]].position) * 50.0).toPointF();
            m_points2d << QPoint(mapped.x() + (SIZE / 2), SIZE - (mapped.y() + 50));
        }
    }
}

CubeClickableFace CubeDrawer::faceAtPos(QPoint pos)
{
    int pi = 0;
    int ci = 0;
    for (auto clickable : clickables) {
        Q_UNUSED(clickable);

        QPoint tr1[3] = { m_points2d[pi++], m_points2d[pi++], m_points2d[pi++] };
        QPoint tr2[3] = { m_points2d[pi++], m_points2d[pi++], m_points2d[pi++] };

        if (
            Utils::triangleDir(tr1[0], tr1[1], tr1[2]) &&
            (Utils::pointInTriangle(pos, tr1[0], tr1[1], tr1[2]) || Utils::pointInTriangle(pos, tr2[0], tr2[1], tr2[2]))
        ) {
            return (CubeClickableFace)ci;
        }

        ci++;
    }

    return CubeClickableFace::None;
}

CubeClickableFace CubeDrawer::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    CubeClickableFace lastFace = CubeClickableFace::None;
    m_faceAtCursor = faceAtPos(pos);
    if (m_faceAtCursor == lastFace) {
        return lastFace;
    }

    for (auto &line : m_lines) {
        line.color = 2;
    }

    if (m_faceAtCursor == CubeClickableFace::None) {
        m_needsUpdateGeometry = true;

        return m_faceAtCursor;
    }

    int fi = (int) m_faceAtCursor;
    int li = 0;

    m_needsUpdateGeometry = true;

    return m_faceAtCursor;
}

void CubeDrawer::leaveEvent(QEvent *event)
{
    for (auto &line : m_lines) {
        line.color = 0;
    }
}

void CubeDrawer::init()
{
    initializeOpenGLFunctions();

    m_fbo = new QOpenGLFramebufferObject(m_width, m_height, QOpenGLFramebufferObject::Depth, GL_TEXTURE_2D);
    m_vao.create();
    m_vbo.create();

    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/cube_vertex.glsl");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/cube_fragment.glsl");
    m_program->link();

    m_copyProgram = new QOpenGLShaderProgram();
    m_copyProgram->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/2dcopy_vertex.glsl");
    m_copyProgram->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/2dcopy_fragment.glsl");
    m_copyProgram->link();

    m_texture = new QOpenGLTexture(QImage(":/images/cube_texture.png").mirrored(true, true));
    m_texture->create();
    m_texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    m_texture->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    m_texture->allocateStorage();
}

void CubeDrawer::initAttributes()
{
    quintptr offset = 0;

    int vertexLocation = m_program->attributeLocation("a_position");
    m_program->enableAttributeArray(vertexLocation);
    m_program->setAttributeBuffer(vertexLocation, GL_FLOAT, offset, 3, sizeof(CubeVertexData));

    offset += sizeof(QVector3D);

    int colorLocation = m_program->attributeLocation("a_color");
    m_program->enableAttributeArray(colorLocation);
    m_program->setAttributeBuffer(colorLocation, GL_FLOAT, offset, 1, sizeof(CubeVertexData));

    offset += sizeof(GLfloat);

    int textureLocation = m_program->attributeLocation("a_texcoord");
    m_program->enableAttributeArray(textureLocation);
    m_program->setAttributeBuffer(textureLocation, GL_FLOAT, offset, 3, sizeof(CubeVertexData));
}

void CubeDrawer::updateGeometry(GLPalette &palette)
{
    if (!m_vbo.isCreated()) {
        init();
    }

    if (m_vao.isCreated()) {
        m_vao.bind();
    }

    m_vbo.bind();

    // const int lineColor = palette.color(0.0, 0.0, 0.0);
    // for (auto clickable : clickables) {
    //     QVector3D ofs = cube[clickable[0]].start * 1;
    //     m_lines
    //         << VertexData(cube[clickable[0]].position + ofs, lineColor, cube[clickable[0]].start)
    //         << VertexData(cube[clickable[1]].position + ofs, lineColor, cube[clickable[0]].start)

    //         << VertexData(cube[clickable[1]].position + ofs, lineColor, cube[clickable[0]].start)
    //         << VertexData(cube[clickable[5]].position + ofs, lineColor, cube[clickable[0]].start)

    //         << VertexData(cube[clickable[5]].position + ofs, lineColor, cube[clickable[0]].start)
    //         << VertexData(cube[clickable[2]].position + ofs, lineColor, cube[clickable[0]].start)

    //         << VertexData(cube[clickable[2]].position + ofs, lineColor, cube[clickable[0]].start)
    //         << VertexData(cube[clickable[0]].position + ofs, lineColor, cube[clickable[0]].start)
    //         ;
    // }

    m_vbo.allocate((m_triangles + m_lines).constData(), (m_triangles.count() + m_lines.count()) * sizeof(CubeVertexData));

    if (m_vao.isCreated()) {
        initAttributes();
        m_vao.release();
    }

    m_vbo.release();

    m_needsUpdateGeometry = false;
}

void CubeDrawer::drawCube(GLPalette &palette)
{
    if (m_needsUpdateGeometry) {
        updateGeometry(palette);
    }

    if (m_vao.isCreated()) {
        m_vao.bind();
    } else {
        m_vbo.bind();
        initAttributes();
    }

    m_program->bind();
    m_program->setUniformValue("u_mvp_matrix", m_projectionMatrix * m_viewMatrix);

    QMatrix4x4 cMatrix;
    cMatrix.scale(0.9);
    m_program->setUniformValue("u_c_matrix", cMatrix);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    m_program->setUniformValue("u_texture", 0);
    m_texture->bind(GL_TEXTURE0);
    glDrawArrays(GL_TRIANGLES, 0, m_triangles.count());
    glDrawArrays(GL_LINES, m_triangles.count(), m_lines.count());

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (m_vao.isCreated()) {
        m_vao.release();
    } else {
        m_vbo.release();
    }

    m_program->release();

    glFlush();
}

void CubeDrawer::setEye(float face)
{
    if (m_faceAtCursor == CubeClickableFace::None) {
        return;
    }
}
