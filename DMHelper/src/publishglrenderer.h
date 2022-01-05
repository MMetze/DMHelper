#ifndef PUBLISHGLRENDERER_H
#define PUBLISHGLRENDERER_H

#include <QObject>

class CampaignObjectBase;
class PublishGLImage;
class QOpenGLWidget;
class QOpenGLFunctions;

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
    virtual void cleanup();
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

    // Public pointer slots
    virtual void pointerToggled(bool enabled);
    virtual void setPointerPosition(const QPointF& pos);
    virtual void setPointerFileName(const QString& filename);

protected:
    virtual void updateProjectionMatrix() = 0;

    // Protected pointer functions
    virtual void paintPointer(QOpenGLFunctions* functions, const QSize& sceneSize, int shaderModelMatrix);
    virtual void setPointerScale(qreal pointerScale);
    virtual void evaluatePointer();
    virtual QPixmap getPointerPixmap();

    QOpenGLWidget* _targetWidget;
    int _rotation;

    PublishGLImage* _pointerImage;
    bool _pointerActive;
    QPointF _pointerPos;
    QString _pointerFile;
};

#endif // PUBLISHGLRENDERER_H
