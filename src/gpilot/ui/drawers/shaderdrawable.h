#ifndef SHADERDRAWABLE_H
#define SHADERDRAWABLE_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include "utils/utils.h"
#include "ui/widgets/glpalette.h"

struct VertexData
{
    VertexData() {}

    VertexData(QVector3D pos, GLfloat col, QVector3D sta) {
        position = pos;
        color = (GLuint) col;
        start = sta;
        this->cumSegPosition = 0;
    }

    VertexData(QVector3D pos, GLuint col, GLfloat cumSegPosition, QVector3D sta) {
        position = pos;
        color = col;
        start = sta;
        this->cumSegPosition = cumSegPosition;
    }

    QVector3D position;
    GLuint color;
    QVector3D start;
    GLfloat cumSegPosition;
};

struct _2DTexturedVertexData
{
    _2DTexturedVertexData() {}
    _2DTexturedVertexData(QVector2D pos, QVector2D tex) {
        position = pos;
        texCoord = tex;
    }

    QVector2D position;
    QVector2D texCoord;
};

class ShaderDrawable : protected QOpenGLFunctions
{
public:
    enum class ProgramType {
        Default,
        GCode,
    };

    explicit ShaderDrawable();
    ~ShaderDrawable();
    void update();
    void draw(QOpenGLShaderProgram *shaderProgram);

    bool needsUpdateGeometry() const;
    virtual void updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette);

    virtual QVector3D getSizes();
    virtual QVector3D getMinimumExtremes();
    virtual QVector3D getMaximumExtremes();
    virtual int getVertexCount();

    double lineWidth() const;
    void setLineWidth(double lineWidth);

    bool visible() const;
    void setVisible(bool visible);
    void toggleVisible();

    double pointSize() const;
    void setPointSize(double pointSize);

    QList<VertexData>& lines() { return m_lines; }
    virtual bool updateData(GLPalette &palette);
    void bindData(QOpenGLShaderProgram *shaderProgram);

    virtual ProgramType programType() { return ProgramType::Default; };

    virtual bool sort(QMatrix4x4 viewMatrix);
protected:
    double m_lineWidth;
    double m_pointSize;
    bool m_visible;

    QVector<VertexData> m_lines;
    QVector<VertexData> m_points;
    QVector<VertexData> m_triangles;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo; // Protected for direct vbo access

    void init();
    virtual void bindAttributes(QOpenGLShaderProgram *&shaderProgram);

private:

    bool m_needsUpdateGeometry;
};

#endif // SHADERDRAWABLE_H
