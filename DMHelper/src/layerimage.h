#ifndef LAYERIMAGE_H
#define LAYERIMAGE_H

#include "layer.h"
#include <QImage>

class QGraphicsPixmapItem;
class PublishGLBattleBackground;

class LayerImage : public Layer
{
    Q_OBJECT
public:
    explicit LayerImage(const QImage& image, int order = 0, QObject *parent = nullptr);
    virtual ~LayerImage() override;

    virtual QRectF boundingRect() const override;

public slots:
    // DM Window Generic Interface
    virtual void dmInitialize(QGraphicsScene& scene) override;
    virtual void dmUninitialize() override;
    virtual void dmUpdate() override;

    // Player Window Generic Interface
    virtual void playerGLInitialize() override;
    virtual void playerGLUninitialize() override;
    //virtual bool playerGLUpdate() override;
    virtual void playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix) override;
    virtual void playerGLResize(int w, int h) override;

    // Local Interface
    void updateImage(const QImage& image);

protected:
    // DM Window Methods
    void cleanupDM();

    // Player Window Methods

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Player Window Members
    PublishGLBattleBackground* _backgroundObject;

    // Core contents
    QImage _layerImage;

};

class LayerImageSourceSignaller : public QObject
{
    Q_OBJECT

public:
    explicit LayerImageSourceSignaller(QObject *parent = nullptr);
    void emitSignal(const QImage& image);

signals:
    void imageChanged(const QImage& image);
};

class ILayerImageSource
{
public:
    explicit ILayerImageSource();
    virtual ~ILayerImageSource();

    virtual const QImage& getImage() const = 0;
    LayerImageSourceSignaller& getSignaller();
    void emitSignal(const QImage& image);

protected:
    LayerImageSourceSignaller _signaller;
};


#endif // LAYERIMAGE_H
