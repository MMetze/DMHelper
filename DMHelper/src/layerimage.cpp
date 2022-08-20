#include "layerimage.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QDebug>

LayerImage::LayerImage(const QImage& image, int order, QObject *parent) :
    Layer{order, parent},
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
    if(_graphicsItem)
    {
        qDebug() << "[LayerImage] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene.addPixmap(QPixmap::fromImage(_layerImage));
    if(_graphicsItem)
    {
        _graphicsItem->setEnabled(false);
        _graphicsItem->setZValue(getOrder());
    }
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

void LayerImage::updateImage(const QImage& image)
{
    if(_layerImage == image)
        return;

    _layerImage = image;
    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(image));
}

void LayerImage::cleanupDM()
{
    if(_graphicsItem)
    {
        if(_graphicsItem->scene())
            _graphicsItem->scene()->removeItem(_graphicsItem);

        delete _graphicsItem;
        _graphicsItem = nullptr;
    }
}




LayerImageSourceSignaller::LayerImageSourceSignaller(QObject *parent) :
    QObject(parent)
{
}

void LayerImageSourceSignaller::emitSignal(const QImage& image)
{
    emit imageChanged(image);
}




ILayerImageSource::ILayerImageSource()
{
}

ILayerImageSource::~ILayerImageSource()
{
}

LayerImageSourceSignaller& ILayerImageSource::getSignaller()
{
    return _signaller;
}

void ILayerImageSource::emitSignal(const QImage& image)
{
    _signaller.emitSignal(image);
}
