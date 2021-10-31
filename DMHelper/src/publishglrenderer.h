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
    virtual void rendererActivated(QOpenGLWidget* glWidget);
    virtual void rendererDeactivated();
    virtual void cleanup() = 0;
    virtual bool deleteOnDeactivation();

    virtual void updateRender();
    virtual void setBackgroundColor(const QColor& color);

    // Standard OpenGL calls
    virtual void initializeGL() = 0;
    virtual void resizeGL(int w, int h) = 0;
    virtual void paintGL() = 0;

signals:
    void updateWidget();
    void deactivated();

protected:
    QOpenGLWidget* _targetWidget;

};

#endif // PUBLISHGLRENDERER_H
