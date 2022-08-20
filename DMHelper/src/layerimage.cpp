#include "layerimage.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>

LayerImage::LayerImage(const QImage& image, QObject *parent) :
    Layer{parent},
    _graphicsItem(nullptr),
    _layerImage(image)
{
}

LayerImage::~LayerImage()
{
    cleanupDM();
}

QRectF LayerImage::boundingRect() const
{
    return _graphicsItem ? _graphicsItem->boundingRect() : QRectF();
}

void LayerImage::dmInitialize(QGraphicsScene& scene)
{
}

void LayerImage::dmUninitialize()
{
    cleanupDM();
}

void LayerImage::dmUpdate()
{
}

void LayerImage::playerGLInitialize()
{
}

void LayerImage::playerGLUninitialize()
{
}

void LayerImage::playerGLUpdate()
{
}

void LayerImage::playerGLPaint()
{
}

void LayerImage::cleanupDM()
{
    if(_graphicsItem)
    {
        if(_graphicsItem->scene())
            _graphicsItem->scene()->removeItem(_graphicsItem);

        delete _graphicsItem;
    }
}




ILayerImageSource::LayerImageSourceSignaller::LayerImageSourceSignaller(QObject *parent) :
    QObject(parent)
{
}

void ILayerImageSource::LayerImageSourceSignaller::emitSignal(const QImage& image)
{
    emit imageChanged(image);
}




ILayerImageSource::ILayerImageSource()
{
}

ILayerImageSource::~ILayerImageSource()
{
}

ILayerImageSource::LayerImageSourceSignaller& ILayerImageSource::getSignaller()
{
    return _signaller;
}

void ILayerImageSource::emitSignal(const QImage& image)
{
    _signaller.emitSignal(image);
}
