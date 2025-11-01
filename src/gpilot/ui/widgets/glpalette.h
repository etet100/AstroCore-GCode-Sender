#ifndef GLPALETTE_H
#define GLPALETTE_H

#include <QtClassHelperMacros>
#include <QOpenGLTexture>
#include <QVector4D>
#include <QMap>
#include <QDebug>

typedef QVector4D GLColor;

class GLPalette
{
    public:
        GLPalette();
        void initialize();
        void bind(GLuint unit = -1);
        void release();
        GLuint color(float r, float g, float b)
        {
            QString index = QString("%1_%2_%3").arg(r).arg(g).arg(b);
            if (m_indexes.contains(index)) {
                return m_indexes[index];
            }

            return add(r, g, b);
        }
        GLuint color(const GLColor& color_)
        {
            QString index = QString("%1_%2_%3").arg(color_.x()).arg(color_.y()).arg(color_.z());
            if (m_indexes.contains(index)) {
                return m_indexes[index];
            }

            return add(color_);
        }
        GLuint color(const QColor& color_)
        {
            return color(color_.redF(), color_.greenF(), color_.blueF());
        }
        GLuint add(float r, float g, float b);
        GLuint add(const GLColor& color);
        int count();
        QString colorAsHex(int index);
        GLPalette& operator << (const GLColor& color);

    private:
        QOpenGLTexture *m_texture;
        QList<GLColor> m_colors;
        QMap<QString, int> m_indexes;
        bool m_updated = false;
        void generateTexture();

    Q_DISABLE_COPY(GLPalette)
};

#endif // GLPALETTE_H
