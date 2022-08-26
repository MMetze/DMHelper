#ifndef LAYERSCENE_H
#define LAYERSCENE_H

#include <QObject>
#include <QList>
#include <QOpenGLFunctions>
#include "dmconstants.h"

class Layer;
class QGraphicsScene;

class LayerScene : public QObject
{
    Q_OBJECT
public:
    explicit LayerScene(QObject *parent = nullptr);
    virtual ~LayerScene();

    virtual QRectF boundingRect() const;
    virtual QSizeF sceneSize() const;

    int layerCount() const;
    Layer* layerAt(int position) const;
    void insertLayer(int position, Layer* layer);
    void prependLayer(Layer* layer);
    void appendLayer(Layer* layer);
    void removeLayer(int position);
    void clearLayers();
    void moveLayer(int from, int to);

    Layer* getFirst(DMHelper::LayerType type) const;
    QImage mergedImage();

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene);
    virtual void dmUninitialize();
    virtual void dmUpdate();

    // Player Window Generic Interface
    virtual void playerGLInitialize();
    virtual void playerGLUninitialize();
    virtual bool playerGLUpdate();
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix);
    virtual void playerGLResize(int w, int h);

signals:

protected:
    QList<Layer*> _layers;

};

#endif // LAYERSCENE_H
