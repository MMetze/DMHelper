#ifndef LAYERSCENE_H
#define LAYERSCENE_H

#include <QObject>
#include <QList>

class Layer;
class QGraphicsScene;

class LayerScene : public QObject
{
    Q_OBJECT
public:
    explicit LayerScene(QObject *parent = nullptr);
    virtual ~LayerScene();

    virtual QRectF boundingRect() const;

    int layerCount();
    Layer* layerAt(int position);
    void insertLayer(int position, Layer* layer);
    void prependLayer(Layer* layer);
    void appendLayer(Layer* layer);
    void removeLayer(int position);
    void clearLayers();
    void moveLayer(int from, int to);

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene);
    virtual void dmUninitialize();
    virtual void dmUpdate();

    // Player Window Generic Interface
    virtual void playerGLInitialize();
    virtual void playerGLUninitialize();
    virtual void playerGLUpdate();
    virtual void playerGLPaint();

signals:

protected:
    QList<Layer*> _layers;

};

#endif // LAYERSCENE_H
