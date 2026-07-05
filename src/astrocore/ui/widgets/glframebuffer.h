#ifndef GLFRAMEBUFFER_H
#define GLFRAMEBUFFER_H

#include <QOpenGLFramebufferObject>
#include <QOpenGLTexture>
#include <QOpenGLFunctions_3_0>

class GLFramebuffer : public QOpenGLFramebufferObject
{
    public:
        GLFramebuffer(int width, int height);
        ~GLFramebuffer();
        bool bind();
        bool release();
        GLuint colorTexture() { return m_colorTexture.textureId(); }
        GLuint depthTexture() { return m_depthTexture.textureId(); }
        void dumpColorTexture(QString filename);
        void dumpDepthTexture(QString filename);
        QString dumpDepthMinMax();

    private:
        QOpenGLTexture m_colorTexture;
        QOpenGLTexture m_depthTexture;
        QOpenGLFunctions_3_0 *m_glfuncs;
        bool m_initialized = false;
};

#endif // GLFRAMEBUFFER_H
