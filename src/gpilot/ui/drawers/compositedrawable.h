#ifndef COMPOSITEDRAWABLE_H
#define COMPOSITEDRAWABLE_H

#include <QList>
#include <memory>
#include "idrawable.h"
#include "shaderdrawable.h"

// Groups multiple ShaderDrawable parts into one logical drawable.
// Each part has its own model matrix and transformation, but they are
// managed together as a single object.
class CompositeDrawable : public IDrawable
{
public:
    CompositeDrawable() = default;
    ~CompositeDrawable() override = default;

    void addPart(std::unique_ptr<ShaderDrawable> part);
    ShaderDrawable* part(int index);
    int partCount() const;

    void update() override;
    void draw(QOpenGLShaderProgram *shaderProgram) override;
    void updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette) override;
    bool needsUpdateGeometry() const override;

    bool visible() const override;
    void setVisible(bool visible) override;
    bool depthTestEnabled() override;

    ProgramType programType() override { return ProgramType::Default; }
    bool sort(QMatrix4x4 viewMatrix) override;

    QVector3D minimumExtremes() override;
    QVector3D maximumExtremes() override;
    void bindData(QOpenGLShaderProgram *shaderProgram) override;
    int getVertexCount() override;

private:
    QList<std::unique_ptr<ShaderDrawable>> m_parts;
    bool m_visible = true;
};

#endif // COMPOSITEDRAWABLE_H
