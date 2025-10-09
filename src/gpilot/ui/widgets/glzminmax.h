#ifndef GLZMINMAX_H
#define GLZMINMAX_H

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions_3_0>

struct MinMax
{
    float min;
    float max;
};

class GLZMinMax : public QOpenGLFunctions_3_0
{
    public:
        GLZMinMax();

    private:
        QOpenGLShaderProgram *m_program;

        void initialize();
        MinMax getMinMax(int count);
        bool m_initialized = false;
};

#endif // GLZMINMAX_H
