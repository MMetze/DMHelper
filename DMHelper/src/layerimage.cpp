#include "layerimage.h"
#include "publishglbattlebackground.h"
#include "dmhfilereader.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLFunctions>
#include <QDebug>

// TODO: Layers - clean up image loading to use the filename

LayerImage::LayerImage(const QString& name, const QString& filename, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _backgroundObject(nullptr),
    _filename(filename),
    _layerImage(),
    _filterApplied(false),
    _filter()
{
    connect(this, &LayerImage::dirty, this, &LayerImage::updateImageInternal);
}

LayerImage::~LayerImage()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerImage::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _filename = element.attribute("imageFile");
    if(_filename.isEmpty()) // Backwards compatibility
        _filename = element.attribute("filename");
    if(_filename == QString(".")) // In case the map file is this trivial, it can be ignored
        _filename.clear();

    QDomElement filterElement = element.firstChildElement(QString("filter"));
    if(!filterElement.isNull())
    {
        _filterApplied = true;

        _filter._r2r = filterElement.attribute("r2r",QString::number(1.0)).toDouble();
        _filter._g2r = filterElement.attribute("g2r",QString::number(0.0)).toDouble();
        _filter._b2r = filterElement.attribute("b2r",QString::number(0.0)).toDouble();
        _filter._r2g = filterElement.attribute("r2g",QString::number(0.0)).toDouble();
        _filter._g2g = filterElement.attribute("g2g",QString::number(1.0)).toDouble();
        _filter._b2g = filterElement.attribute("b2g",QString::number(0.0)).toDouble();
        _filter._r2b = filterElement.attribute("r2b",QString::number(0.0)).toDouble();
        _filter._g2b = filterElement.attribute("g2b",QString::number(0.0)).toDouble();
        _filter._b2b = filterElement.attribute("b2b",QString::number(1.0)).toDouble();
        _filter._sr = filterElement.attribute("sr",QString::number(1.0)).toDouble();
        _filter._sg = filterElement.attribute("sg",QString::number(1.0)).toDouble();
        _filter._sb = filterElement.attribute("sb",QString::number(1.0)).toDouble();

        _filter._isOverlay = static_cast<bool>(filterElement.attribute("isOverlay",QString::number(1)).toInt());
        _filter._overlayColor.setNamedColor(filterElement.attribute("overlayColor",QString("#000000")));
        _filter._overlayAlpha = filterElement.attribute("overlayAlpha",QString::number(128)).toInt();
    }

    Layer::inputXML(element, isImport);
}

QRectF LayerImage::boundingRect() const
{
    //return _graphicsItem ? _graphicsItem->boundingRect() : QRectF();
    return _layerImage.isNull() ? QRectF() : QRectF(_layerImage.rect());
}

QImage LayerImage::getLayerIcon() const
{
    return getImage();
}

DMHelper::LayerType LayerImage::getType() const
{
    return DMHelper::LayerType_Image;
}

Layer* LayerImage::clone() const
{
    LayerImage* newLayer = new LayerImage(_name, _filename, _order);

    newLayer->_layerVisible = _layerVisible;
    newLayer->_filterApplied = _filterApplied;
    newLayer->_filter = _filter;

    return newLayer;
}

QString LayerImage::getImageFile() const
{
    return _filename;
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

void LayerImage::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if(!functions)
        return;

    if(!_backgroundObject)
    {
        playerGLInitialize();
        if(!_backgroundObject)
            return;
    }

    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
    _backgroundObject->paintGL();
}

void LayerImage::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void LayerImage::initialize(const QSize& layerSize)
{
    DMHFileReader* reader = new DMHFileReader(getImageFile());
    if(reader)
    {
        _layerImage = reader->loadImage();
        if(!_layerImage.isNull())
        {
            _filename = reader->getFilename();
            if(!layerSize.isEmpty())
                _layerImage = _layerImage.scaled(layerSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        delete reader;
    }
}

void LayerImage::uninitialize()
{
    _layerImage = QImage();
}

void LayerImage::setOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);

    Layer::setOrder(order);
}

void LayerImage::setLayerVisible(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);

    Layer::setLayerVisible(layerVisible);
}

void LayerImage::updateImage(const QImage& image)
{
    if(_layerImage == image)
        return;

    _layerImage = image;
    emit dirty();
    emit imageChanged(getImage());
}

void LayerImage::setFileName(const QString& filename)
{
    if((filename.isEmpty()) || (_filename == filename))
        return;

    cleanupDM();
    cleanupPlayer();
    _filename = filename;
    emit dirty();
    emit imageChanged(getImage()); // TODO: Layers - make sure this really shares the updated image
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

void LayerImage::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("imageFile", targetDirectory.relativeFilePath(_filename));

    if(_filterApplied)
    {
        QDomElement filterElement = doc.createElement("filter");
        filterElement.setAttribute("r2r", _filter._r2r);
        filterElement.setAttribute("g2r", _filter._g2r);
        filterElement.setAttribute("b2r", _filter._b2r);
        filterElement.setAttribute("r2g", _filter._r2g);
        filterElement.setAttribute("g2g", _filter._g2g);
        filterElement.setAttribute("b2g", _filter._b2g);
        filterElement.setAttribute("r2b", _filter._r2b);
        filterElement.setAttribute("g2b", _filter._g2b);
        filterElement.setAttribute("b2b", _filter._b2b);
        filterElement.setAttribute("sr", _filter._sr);
        filterElement.setAttribute("sg", _filter._sg);
        filterElement.setAttribute("sb", _filter._sb);
        filterElement.setAttribute("isOverlay", _filter._isOverlay);
        filterElement.setAttribute("overlayColor", _filter._overlayColor.name());
        filterElement.setAttribute("overlayAlpha", _filter._overlayAlpha);
        element.appendChild(filterElement);
    }

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
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
