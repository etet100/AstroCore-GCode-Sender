#ifndef SHADERDRAWABLE_H
#define SHADERDRAWABLE_H

#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLTexture>
#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>
#include "utils/utils.h"
#include "ui/widgets/glpalette.h"
#include "abstractdrawable.h"

#define QUANTIZE_COLOR_STEPS 25
#define QUANTIZE_COLOR(x) ((int) floor(x * QUANTIZE_COLOR_STEPS) / QUANTIZE_COLOR_STEPS)

struct VertexData
{
    VertexData() {}

    VertexData(QVector3D pos, GLfloat col) {
        position = pos;
        color = (GLuint) col;
        this->cumSegPosition = 0;
    }

    VertexData(QVector3D pos, GLfloat col, GLfloat cumSegPosition) {
        position = pos;
        color = (GLuint) col;
        this->cumSegPosition = cumSegPosition;
    }

    VertexData(QVector3D pos, GLfloat col, QVector3D sta) {
        position = pos;
        color = (GLuint) col;
        start = sta;
        this->cumSegPosition = 0;
    }

    VertexData(QVector3D pos, GLuint col, GLfloat cumSegPosition, QVector3D sta) {
        position = pos;
        color = (GLuint) col;
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

class ShaderDrawable : public AbstractDrawable, protected QOpenGLFunctions
{
public:
    explicit ShaderDrawable();
    ~ShaderDrawable();
    void update() override;
    virtual void draw(QOpenGLShaderProgram *shaderProgram) override;

    bool needsUpdateGeometry() const override;
    virtual void updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette) override;

    virtual QVector3D sizes();
    virtual QVector3D minimumExtremes() override;
    virtual QVector3D maximumExtremes() override;
    virtual int getVertexCount() override;

    double lineWidth() const;
    void setLineWidth(double lineWidth);

    bool visible() const override;
    void setVisible(bool visible) override;
    void toggleVisible();
    bool depthTestEnabled() override { return m_depthTestEnabled; }
    void setPointSize(double pointSize);

    void setTranslation(const QVector3D &translation);
    void setRotation(float angle, const QVector3D &axis);
    void setRotation(float x, float y, float z);
    void setOrigin(const QVector3D &origin);
    const QMatrix4x4& modelMatrix() const;

    QList<VertexData>& lines() { return m_lines; }
    virtual bool updateData(GLPalette &palette);
    void bindData(QOpenGLShaderProgram *shaderProgram) override;

    virtual ProgramType programType() override { return ProgramType::Default; };

    virtual bool sort(QMatrix4x4 viewMatrix) override;

protected:
    double m_lineWidth = 1.0;
    double m_pointSize = 4.0;
    bool m_visible = true;
    bool m_needsUpdateGeometry = true;
    bool m_flatShading = false;
    bool m_depthTestEnabled = true;

    QVector<VertexData> m_lines;
    QVector<VertexData> m_points;
    QVector<VertexData> m_triangles;

    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo; // Protected for direct vbo access

    QMatrix4x4 m_modelMatrix;
    QVector3D m_translation;
    QQuaternion m_rotation;
    QVector3D m_origin;

    void init();
    // it has to be called if asynchronous update of geometry is used
    void updateVerticesBuffers();
    virtual void bindAttributes(QOpenGLShaderProgram *&shaderProgram);

private:
    void rebuildModelMatrix();
};

#endif // SHADERDRAWABLE_H
