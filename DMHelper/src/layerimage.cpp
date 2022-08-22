#include "layerimage.h"
#include "publishglbattlebackground.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLFunctions>
#include <QDebug>

LayerImage::LayerImage(const QImage& image, int order, QObject *parent) :
    Layer{order, parent},
    _graphicsItem(nullptr),
    _backgroundObject(nullptr),
    _layerImage(image)
{
    if(_layerImage.isNull())
        qDebug() << "[LayerImage] ERROR: layer image created with null image!";
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
    if(_backgroundObject)
    {
        qDebug() << "[LayerImage] ERROR: playerGLInitialize called although the background object already exists!";
        return;
    }

    _backgroundObject = new PublishGLBattleBackground(nullptr, _layerImage, GL_NEAREST);
}

void LayerImage::playerGLUninitialize()
{
    delete _backgroundObject;
    _backgroundObject = nullptr;
}

/*
bool LayerImage::playerGLUpdate()
{
    return false;
}
*/

void LayerImage::playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix)
{
    if((!_backgroundObject) || (!functions))
        return;

    functions->glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

void LayerImage::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
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
