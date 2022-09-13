#include "layerimage.h"
#include "publishglbattlebackground.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLFunctions>
#include <QDebug>

LayerImage::LayerImage(const QString& name, const QImage& image, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _backgroundObject(nullptr),
    _layerImage(image),
    _filterApplied(false),
    _filter()
{
    if(_layerImage.isNull())
        qDebug() << "[LayerImage] ERROR: layer image created with null image!";

    connect(this, &LayerImage::dirty, this, &LayerImage::updateImageInternal);
}

LayerImage::~LayerImage()
{
    cleanupDM();
    cleanupPlayer();
}

QRectF LayerImage::boundingRect() const
{
    return _graphicsItem ? _graphicsItem->boundingRect() : QRectF();
}

QImage LayerImage::getLayerIcon() const
{
    return getImage();
}

DMHelper::LayerType LayerImage::getType() const
{
    return DMHelper::LayerType_Image;
}

QImage LayerImage::getImage() const
{
    return _filterApplied ? _filter.apply(_layerImage) : _layerImage;
}

QImage LayerImage::getImageUnfiltered() const
{
    return _layerImage;
}

bool LayerImage::isFilterApplied() const
{
    return _filterApplied;
}

MapColorizeFilter LayerImage::getFilter() const
{
    return _filter;
}

void LayerImage::dmInitialize(QGraphicsScene& scene)
{
    qDebug() << "[LayerImage] Image Layer being initialized...";
    if(_graphicsItem)
    {
        qDebug() << "[LayerImage] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene.addPixmap(QPixmap::fromImage(getImage()));
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

    _backgroundObject = new PublishGLBattleBackground(nullptr, getImage(), GL_NEAREST);
}

void LayerImage::playerGLUninitialize()
{
    cleanupPlayer();
}

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
    emit dirty();
    emit imageChanged(getImage());
}

void LayerImage::setApplyFilter(bool applyFilter)
{
    if(_filterApplied == applyFilter)
        return;

    _filterApplied = applyFilter;
    emit dirty();
    emit imageChanged(getImage());
}

void LayerImage::setFilter(const MapColorizeFilter& filter)
{
    if(filter == _filter)
        return;

    _filter = filter;
    emit dirty();
    emit imageChanged(getImage());
}

void LayerImage::updateImageInternal()
{
    QImage newImage = getImage();

    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(newImage));

    if(_backgroundObject)
        _backgroundObject->setImage(newImage);
}

void LayerImage::cleanupDM()
{
    qDebug() << "[LayerImage] Cleaning up Image Layer...";
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerImage::cleanupPlayer()
{
    delete _backgroundObject;
    _backgroundObject = nullptr;
}
