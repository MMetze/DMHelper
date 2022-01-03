#ifndef PUBLISHGLRENDERER_H
#define PUBLISHGLRENDERER_H

#include <QObject>

class CampaignObjectBase;
class QOpenGLWidget;

class PublishGLRenderer : public QObject
{
    Q_OBJECT

public:
    PublishGLRenderer(QObject *parent = nullptr);
    virtual ~PublishGLRenderer();

    virtual CampaignObjectBase* getObject();
    virtual QUuid getObjectId();
    virtual QColor getBackgroundColor();

    // DMH OpenGL renderer calls
    virtual void rendererActivated(QOpenGLWidget* glWidget);
    virtual void rendererDeactivated();
    virtual void cleanup() = 0;
    virtual bool deleteOnDeactivation();

    virtual void updateRender();

    // Standard OpenGL calls
    virtual void initializeGL() = 0;
    virtual void resizeGL(int w, int h) = 0;
    virtual void paintGL() = 0;

signals:
    void updateWidget();
    void deactivated();

public slots:
    virtual void setBackgroundColor(const QColor& color);
    virtual void setRotation(int rotation);

protected:
    virtual void updateProjectionMatrix();

    QOpenGLWidget* _targetWidget;
    int _rotation;
};

#endif // PUBLISHGLRENDERER_H
