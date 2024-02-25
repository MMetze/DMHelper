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
    _imageGLObject(nullptr),
    _scene(nullptr),
    _filename(filename),
    _originalImage(),
    _layerImage(),
    _filterApplied(false),
    _filter()
{
}

LayerImage::~LayerImage()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerImage::inputXML(const QDomElement &element, bool isImport)
{
    _filename = element.attribute("imageFile");
    if(_filename.isEmpty()) // Backwards compatibility
        _filename = element.attribute("filename");
    if(_filename == QString(".")) // In case the map file is this trivial, it can be ignored
        _filename.clear();

    QDomElement filterElement = element.firstChildElement(QString("filter"));
    if(!filterElement.isNull())
    {
        _filterApplied = true;

        _filter._r2r = filterElement.attribute("r2r", QString::number(1.0)).toDouble();
        _filter._g2r = filterElement.attribute("g2r", QString::number(0.0)).toDouble();
        _filter._b2r = filterElement.attribute("b2r", QString::number(0.0)).toDouble();
        _filter._r2g = filterElement.attribute("r2g", QString::number(0.0)).toDouble();
        _filter._g2g = filterElement.attribute("g2g", QString::number(1.0)).toDouble();
        _filter._b2g = filterElement.attribute("b2g", QString::number(0.0)).toDouble();
        _filter._r2b = filterElement.attribute("r2b", QString::number(0.0)).toDouble();
        _filter._g2b = filterElement.attribute("g2b", QString::number(0.0)).toDouble();
        _filter._b2b = filterElement.attribute("b2b", QString::number(1.0)).toDouble();
        _filter._sr = filterElement.attribute("sr", QString::number(1.0)).toDouble();
        _filter._sg = filterElement.attribute("sg", QString::number(1.0)).toDouble();
        _filter._sb = filterElement.attribute("sb", QString::number(1.0)).toDouble();

        _filter._isOverlay = static_cast<bool>(filterElement.attribute("isOverlay", QString::number(1)).toInt());
        _filter._overlayColor.setNamedColor(filterElement.attribute("overlayColor", QString("#000000")));
        _filter._overlayAlpha = filterElement.attribute("overlayAlpha", QString::number(128)).toInt();
    }

    Layer::inputXML(element, isImport);
}

QRectF LayerImage::boundingRect() const
{
    return _layerImage.isNull() ? QRectF() : QRectF(_position, _layerImage.size());
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

    copyBaseValues(newLayer);
    newLayer->_filterApplied = _filterApplied;
    newLayer->_filter = _filter;

    return newLayer;
}

void LayerImage::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerImage::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerImage::applyLayerVisiblePlayer(bool layerVisible)
{
    Q_UNUSED(layerVisible);
}

void LayerImage::applyOpacity(qreal opacity)
{
    _opacityReference = opacity;

    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

void LayerImage::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);

    if(_imageGLObject)
    {
        QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        //_imageGLObject->setPosition(QPoint(position.x() + pointTopLeft.x(), pointTopLeft.y() - _size.height() - position.y()));
//        _imageGLObject->setPosition(QPoint(position.x() + pointTopLeft.x(),
//                                           position.y()));
                                           //pointTopLeft.y() - position.y()));
//        _imageGLObject->setPosition(QPoint(0, 0));
//        _imageGLObject->setPosition(position);
        _imageGLObject->setPosition(QPoint(pointTopLeft.x() + position.x(), -pointTopLeft.y() - position.y()));
    }
}

void LayerImage::applySize(const QSize& size)
{
    if(_layerImage.size() == size)
        return;

    _layerImage = _originalImage.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QImage newImage = getImage();
    emit dirty();
    emit imageChanged(newImage);

    updateImageInternal(newImage);

    if(_imageGLObject)
        _imageGLObject->setTargetSize(size);
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

void LayerImage::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    if(_graphicsItem)
    {
        qDebug() << "[LayerImage] ERROR: dmInitialize called although the graphics item already exists!";
        return;
    }

    _graphicsItem = scene->addPixmap(QPixmap::fromImage(getImage()));
    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
    }

    Layer::dmInitialize(scene);
}

void LayerImage::dmUninitialize()
{
    cleanupDM();
}

void LayerImage::dmUpdate()
{
}

void LayerImage::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    if(_imageGLObject)
    {
        qDebug() << "[LayerImage] ERROR: playerGLInitialize called although the background object already exists!";
        return;
    }

    _scene = scene;

    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();

    f->glUseProgram(_shaderProgramRGBA);
    f->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture

    _imageGLObject = new PublishGLBattleBackground(nullptr, getImage(), GL_NEAREST);

    Layer::playerGLInitialize(renderer, scene);
}

void LayerImage::playerGLUninitialize()
{
    cleanupPlayer();
}

void LayerImage::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    if((_shaderProgramRGBA == 0) || (_shaderModelMatrixRGBA == -1) || (_shaderAlphaRGBA == -1))
    {
        qDebug() << "[LayerImage] ERROR: invalid shaders set! _shaderProgramRGBA: " << _shaderProgramRGBA << ", _shaderProjectionMatrixRGBA: " << _shaderProjectionMatrixRGBA << ", _shaderModelMatrixRGBA: " << _shaderModelMatrixRGBA << ", _shaderAlphaRGBA: " << _shaderAlphaRGBA;
        return;
    }

    functions->glUseProgram(_shaderProgramRGBA);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _imageGLObject->getMatrixData());
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

    _imageGLObject->paintGL(functions, projectionMatrix);

    functions->glUseProgram(_shaderProgramRGB);
}

void LayerImage::playerGLResize(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

bool LayerImage::playerIsInitialized()
{
    return _imageGLObject != nullptr;
}

void LayerImage::initialize(const QSize& sceneSize)
{
    Q_UNUSED(sceneSize);

    if(!_layerImage.isNull())
        return;

    DMHFileReader* reader = new DMHFileReader(getImageFile());
    if(reader)
    {
        _originalImage = reader->loadImage();
        if(!_originalImage.isNull())
        {
            _filename = reader->getFilename();
            _layerImage = _originalImage;
        }
        delete reader;
    }

    if(_size.isEmpty())
        setSize(_layerImage.size());
}

void LayerImage::uninitialize()
{
    _layerImage = QImage();
    _originalImage = QImage();
}

void LayerImage::updateImage(const QImage& image)
{
    if(_originalImage == image)
        return;

    // TODO: Layers
    _originalImage = image;
    _layerImage = _originalImage;
    _size = _layerImage.size();

    QImage newImage = getImage();
    emit dirty();
    emit imageChanged(newImage);
    changeImageInternal(newImage);
}

void LayerImage::setFileName(const QString& filename)
{
    if((filename.isEmpty()) || (_filename == filename))
        return;

    cleanupDM();
    cleanupPlayer();
    _filename = filename;

    QImage newImage = getImage();
    emit dirty();
    emit imageChanged(newImage); // TODO: Layers - make sure this really shares the updated image
    changeImageInternal(newImage);
}

void LayerImage::setApplyFilter(bool applyFilter)
{
    if(_filterApplied == applyFilter)
        return;

    _filterApplied = applyFilter;
    QImage newImage = getImage();
    emit dirty();
    emit imageChanged(newImage);
    changeImageInternal(newImage);
}

void LayerImage::setFilter(const MapColorizeFilter& filter)
{
    if(filter == _filter)
        return;

    _filter = filter;
    QImage newImage = getImage();
    emit dirty();
    emit imageChanged(newImage);
    changeImageInternal(newImage);
}

void LayerImage::updateImageInternal(const QImage& newImage)
{
    if(_graphicsItem)
        _graphicsItem->setPixmap(QPixmap::fromImage(newImage));
}

void LayerImage::changeImageInternal(const QImage& newImage)
{
    updateImageInternal(newImage);

    if(_imageGLObject)
        _imageGLObject->setImage(newImage);
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
    if(!_graphicsItem)
        return;

    if(_graphicsItem->scene())
        _graphicsItem->scene()->removeItem(_graphicsItem);

    delete _graphicsItem;
    _graphicsItem = nullptr;
}

void LayerImage::cleanupPlayer()
{
    delete _imageGLObject;
    _imageGLObject = nullptr;

    _scene = nullptr;
}
