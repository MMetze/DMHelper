#ifndef PUBLISHGLRENDERER_H
#define PUBLISHGLRENDERER_H

#include <QObject>

class QOpenGLWidget;

class PublishGLRenderer : public QObject
{
    Q_OBJECT

public:
    PublishGLRenderer(QObject *parent = nullptr);
    virtual ~PublishGLRenderer();

    // DMH OpenGL renderer calls
    virtual void rendererActivated(QOpenGLWidget* glWidget) = 0;
    virtual void rendererDeactivated() = 0;
    virtual void cleanup() = 0;

    // Standard OpenGL calls
    virtual void initializeGL() = 0;
    virtual void resizeGL(int w, int h) = 0;
    virtual void paintGL() = 0;

signals:
    void updateWidget();
};

#endif // PUBLISHGLRENDERER_H
