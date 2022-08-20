#ifndef LAYERIMAGE_H
#define LAYERIMAGE_H

#include "layer.h"
#include <QImage>

class QGraphicsPixmapItem;

class LayerImage : public Layer
{
    Q_OBJECT
public:
    explicit LayerImage(const QImage& image, QObject *parent = nullptr);
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
    virtual void playerGLUpdate() override;
    virtual void playerGLPaint() override;

protected:
    // DM Window Methods
    void cleanupDM();

    // Player Window Methods

    // DM Window Members
    QGraphicsPixmapItem* _graphicsItem;

    // Core contents
    QImage _layerImage;

};

class ILayerImageSource
{
public:
    class LayerImageSourceSignaller : QObject
    {
        Q_OBJECT

    public:
        explicit LayerImageSourceSignaller(QObject *parent = nullptr);
        void emitSignal(const QImage& image);

    signals:
        void imageChanged(const QImage& image);
    };

    explicit ILayerImageSource();
    virtual ~ILayerImageSource();

    virtual const QImage& getImage() const = 0;
    LayerImageSourceSignaller& getSignaller();
    void emitSignal(const QImage& image);

protected:
    LayerImageSourceSignaller _signaller;
};


#endif // LAYERIMAGE_H
