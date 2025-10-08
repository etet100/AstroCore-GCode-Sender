#include "glzminmax.h"

#include <QDebug>

GLZMinMax::GLZMinMax() : QOpenGLFunctions_3_0()
{
}

void GLZMinMax::initialize()
{
    initializeOpenGLFunctions();
    m_program = new QOpenGLShaderProgram();
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/zminmax_vertex.glsl");
    m_program->link();
    m_initialized = true;
}

MinMax GLZMinMax::getMinMax(int count)
{
    if (!m_initialized) {
        initialize();
    }

    GLuint tfBuffer;
    glGenBuffers(1, &tfBuffer);
    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfBuffer);
    glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, count * sizeof(float), nullptr, GL_DYNAMIC_READ);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tfBuffer);

    const char* varyings[] = { "zCamera" };
    glTransformFeedbackVaryings(m_program->programId(), 1, varyings, GL_INTERLEAVED_ATTRIBS);

    glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, tfBuffer);
    float* zData = (float*)glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);

    float zMin = std::numeric_limits<float>::max();
    float zMax = std::numeric_limits<float>::lowest();

    for (size_t i = 0; i < count; i++) {
        zMin = std::min(zMin, zData[i]);
        zMax = std::max(zMax, zData[i]);
    }

    glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

    qDebug() << "Near: " << zMin << ", Far: " << zMax;
}
