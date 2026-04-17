#include "compositedrawable.h"
#include <limits>

void CompositeDrawable::addPart(std::unique_ptr<ShaderDrawable> part)
{
    m_parts.append(std::move(part));
}

ShaderDrawable* CompositeDrawable::part(int index)
{
    return m_parts.at(index).get();
}

int CompositeDrawable::partCount() const
{
    return m_parts.size();
}

void CompositeDrawable::update()
{
    for (auto &p : m_parts) {
        p->update();
    }
}

void CompositeDrawable::draw(QOpenGLShaderProgram *shaderProgram)
{
    if (!m_visible) {
        return;
    }

    for (auto &p : m_parts) {
        p->draw(shaderProgram);
    }
}

void CompositeDrawable::updateGeometry(QOpenGLShaderProgram *shaderProgram, GLPalette &palette)
{
    for (auto &p : m_parts) {
        if (p->needsUpdateGeometry()) {
            p->updateGeometry(shaderProgram, palette);
        }
    }
}

bool CompositeDrawable::needsUpdateGeometry() const
{
    for (const auto &p : m_parts) {
        if (p->needsUpdateGeometry()) {
            return true;
        }
    }

    return false;
}

bool CompositeDrawable::visible() const
{
    return m_visible;
}

void CompositeDrawable::setVisible(bool visible)
{
    m_visible = visible;

    for (auto &p : m_parts) {
        p->setVisible(visible);
    }
}

bool CompositeDrawable::depthTestEnabled()
{
    // Composite uses depth test if any part uses it
    for (const auto &p : m_parts) {
        if (p->depthTestEnabled()) {
            return true;
        }
    }

    return false;
}

bool CompositeDrawable::sort(QMatrix4x4 viewMatrix)
{
    bool resorted = false;
    for (auto &p : m_parts) {
        resorted |= p->sort(viewMatrix);
    }

    return resorted;
}

QVector3D CompositeDrawable::minimumExtremes()
{
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();

    for (const auto &p : m_parts) {
        QVector3D e = p->minimumExtremes();
        minX = qMin(minX, e.x());
        minY = qMin(minY, e.y());
        minZ = qMin(minZ, e.z());
    }

    return QVector3D(minX, minY, minZ);
}

QVector3D CompositeDrawable::maximumExtremes()
{
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const auto &p : m_parts) {
        QVector3D e = p->maximumExtremes();
        maxX = qMax(maxX, e.x());
        maxY = qMax(maxY, e.y());
        maxZ = qMax(maxZ, e.z());
    }

    return QVector3D(maxX, maxY, maxZ);
}

void CompositeDrawable::bindData(QOpenGLShaderProgram *shaderProgram)
{
    for (auto &p : m_parts) {
        p->bindData(shaderProgram);
    }
}

int CompositeDrawable::getVertexCount()
{
    int total = 0;
    for (const auto &p : m_parts) {
        total += p->getVertexCount();
    }

    return total;
}
