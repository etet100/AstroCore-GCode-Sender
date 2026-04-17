#ifndef IDRAWABLE_H
#define IDRAWABLE_H

#include <QOpenGLShaderProgram>
#include <QMatrix4x4>
#include <QVector3D>
#include "ui/widgets/glpalette.h"

class IDrawable
{
public:
    enum class ProgramType {
        Default,
        GCode,
        Billboard,
    };

    virtual ~IDrawable() = default;

    virtual void update() = 0;
    virtual void draw(QOpenGLShaderProgram *shaderProgram) = 0;
    virtual void updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette) = 0;
    virtual bool needsUpdateGeometry() const = 0;

    virtual bool visible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual bool depthTestEnabled() = 0;

    virtual ProgramType programType() = 0;
    virtual bool sort(QMatrix4x4 viewMatrix) = 0;

    virtual QVector3D minimumExtremes() = 0;
    virtual QVector3D maximumExtremes() = 0;

    virtual void bindData(QOpenGLShaderProgram *shaderProgram) = 0;
    virtual int getVertexCount() = 0;
};

#endif // IDRAWABLE_H
