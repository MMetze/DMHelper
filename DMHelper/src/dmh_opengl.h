#ifndef DMH_OPENGL_H
#define DMH_OPENGL_H

#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QOpenGLContext>

#define DMH_DEBUG_OPENGL 0

#if DMH_DEBUG_OPENGL

    #define DMH_DEBUG_OPENGL_FRAME_START() qDebug() << "[DMH_DEBUG_OPENGL] Frame Start (Context: " << QOpenGLContext::currentContext() << ") in " << __FILE__ << ":" << __LINE__;
    #define DMH_DEBUG_OPENGL_PAINTGL() qDebug() << "[DMH_DEBUG_OPENGL] paintGL() (Context: " << QOpenGLContext::currentContext() << ") in " << __FILE__ << ":" << __LINE__;
    #define DMH_DEBUG_OPENGL_glCreateProgram(x, y) qDebug() << "[DMH_DEBUG_OPENGL] glCreateProgram(" << x << ") for " << y << " in " << __FILE__ << ":" << __LINE__;
    #define DMH_DEBUG_OPENGL_glUseProgram(x) qDebug() << "[DMH_DEBUG_OPENGL] glUseProgram(" << x << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::setProgram(x);
    #define DMH_DEBUG_OPENGL_glUniformMatrix4fv4(x, y, z, w) qDebug() << "[DMH_DEBUG_OPENGL] glUniformMatrix4fv(" << x << ", " << y << ", " << z << ", " << w << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glUniformMatrix4fv(x, y, z, w, m) qDebug() << "[DMH_DEBUG_OPENGL] glUniformMatrix4fv(" << x << ", " << y << ", " << z << ", " << w << ") with (" << m.column(3) << "), (" << m.row(0)[0] << ", " << m.row(1)[1] << ", " << m.row(2)[2] << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glUniform1i(x, y) qDebug() << "[DMH_DEBUG_OPENGL] glUniform1i(" << x << ", " << y << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glUniform1f(x, y) qDebug() << "[DMH_DEBUG_OPENGL] glUniform1f(" << x << ", " << y << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glUniform2f(x, y, z) qDebug() << "[DMH_DEBUG_OPENGL] glUniform2f(" << x << ", " << y << ", " << z << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glUniform3f(x, y, z, w) qDebug() << "[DMH_DEBUG_OPENGL] glUniform3f(" << x << ", " << y << ", " << z << ", " << w << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glUniform4f(x, y, z, w, v) qDebug() << "[DMH_DEBUG_OPENGL] glUniform4f(" << x << ", " << y << ", " << z << ", " << w << ", " << v << ") in " << __FILE__ << ":" << __LINE__; DMH_DEBUG_OPENGL_Singleton::checkUniform(x);
    #define DMH_DEBUG_OPENGL_glActiveTexture(x) qDebug() << "[DMH_DEBUG_OPENGL] glActiveTexture(" << x << ") in " << __FILE__ << ":" << __LINE__;
    #define DMH_DEBUG_OPENGL_glBindTexture(x, y) qDebug() << "[DMH_DEBUG_OPENGL] glBindTexture(" << x << ", " << y << ") in " << __FILE__ << ":" << __LINE__;

    #include <QHash>
    #include <QList>

    class DMH_DEBUG_OPENGL_Singleton
    {
    public:
        explicit DMH_DEBUG_OPENGL_Singleton();
        ~DMH_DEBUG_OPENGL_Singleton();

        static void Initialize();
        static void Shutdown();

        static void setProgram(GLuint program);
        static void removeProgram(GLuint program);
        static void registerUniform(GLuint program, GLuint uniform, const char* name);
        static void checkUniform(GLuint uniform);

    private:
        static DMH_DEBUG_OPENGL_Singleton* _instance;

        QHash<GLuint, QList<GLuint>> _uniformHash;
        GLuint _currentProgram;
    };

#else

    #define DMH_DEBUG_OPENGL_FRAME_START()
    #define DMH_DEBUG_OPENGL_PAINTGL()
    #define DMH_DEBUG_OPENGL_glCreateProgram(x, y)
    #define DMH_DEBUG_OPENGL_glUseProgram(x)
    #define DMH_DEBUG_OPENGL_glUniformMatrix4fv4(x, y, z, w)
    #define DMH_DEBUG_OPENGL_glUniformMatrix4fv(x, y, z, w, m)
    #define DMH_DEBUG_OPENGL_glUniform1i(x, y)
    #define DMH_DEBUG_OPENGL_glUniform1f(x, y)
    #define DMH_DEBUG_OPENGL_glUniform2f(x, y, z)
    #define DMH_DEBUG_OPENGL_glUniform3f(x, y, z, w)
    #define DMH_DEBUG_OPENGL_glUniform4f(x, y, z, w, v)
    #define DMH_DEBUG_OPENGL_glActiveTexture(x)
    #define DMH_DEBUG_OPENGL_glBindTexture(x, y)

    class DMH_DEBUG_OPENGL_Singleton
    {
    public:
        explicit DMH_DEBUG_OPENGL_Singleton() {}
        ~DMH_DEBUG_OPENGL_Singleton() {}

        static void Initialize() {}
        static void Shutdown() {}

        static void setProgram(GLuint program) {Q_UNUSED(program);}
        static void removeProgram(GLuint program) {Q_UNUSED(program);}
        static void registerUniform(GLuint program, GLuint uniform, const char* name) {Q_UNUSED(program); Q_UNUSED(uniform); Q_UNUSED(name);}
        static void checkUniform(GLuint uniform) {Q_UNUSED(uniform);}
    };

#endif



#endif // DMH_OPENGL_H
