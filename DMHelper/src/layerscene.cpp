#include "layerscene.h"
#include "layer.h"
#include "layerimage.h"
#include "layerfow.h"
#include "layergrid.h"
#include "layertokens.h"
#include "layerreference.h"
#include "layervideo.h"
#include "layervideotransparent.h"
#include "layerblank.h"
#include "publishglscene.h"
#include "campaign.h"
#include <QRectF>
#include <QImage>
#include <QPainter>
#include <QDomElement>
#include <QDebug>

//#define DEBUG_LAYERSCENE

LayerScene::LayerScene(QObject *parent) :
    CampaignObjectBase{QString(), parent},
    _initialized(false),
    _layers(),
    _scale(0),
    _selected(-1),
    _dmScene(nullptr),
    _playerGLScene(nullptr),
    _renderer(nullptr)
{
}

LayerScene::~LayerScene()
{
    clearLayers();
}

void LayerScene::inputXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(isImport);

    _selected = element.attribute("selected", QString::number(0)).toInt();
    _scale = element.attribute("scale", QString::number(DMHelper::STARTING_GRID_SCALE)).toInt();

    QDomElement layerElement = element.firstChildElement(QString("layer"));
    while(!layerElement.isNull())
    {
        Layer* newLayer = nullptr;
        int layerType = layerElement.attribute(QString("type")).toInt();
        switch(layerType)
        {
            case DMHelper::LayerType_Image:
                newLayer = new LayerImage();
                break;
            case DMHelper::LayerType_Fow:
                newLayer = new LayerFow();
                break;
            case DMHelper::LayerType_Grid:
                newLayer = new LayerGrid();
                break;
            case DMHelper::LayerType_Tokens:
                newLayer = new LayerTokens();
                break;
            case DMHelper::LayerType_Reference:
                newLayer = new LayerReference();
                break;
            case DMHelper::LayerType_Video:
                newLayer = new LayerVideo();
                break;
            case DMHelper::LayerType_VideoTransparent:
                newLayer = new LayerVideoTransparent();
                break;
            case DMHelper::LayerType_Blank:
                newLayer = new LayerBlank();
                break;
            default:
                qDebug() << "[LayerScene] ERROR: unable to read layer for unexpected type: " << layerType;
                break;
        }

        if(newLayer)
        {
            newLayer->setScale(_scale);
            newLayer->inputXML(layerElement, isImport);
            newLayer->setLayerScene(this);
            connectLayer(newLayer);
            //connect(newLayer, &Layer::dirty, this, &LayerScene::dirty);
            _layers.append(newLayer);
        }

        layerElement = layerElement.nextSiblingElement(QString("layer"));
    }
}

void LayerScene::postProcessXML(const QDomElement &element, bool isImport)
{
    Q_UNUSED(element);

    Campaign* campaign = dynamic_cast<Campaign*>(getParentByType(DMHelper::CampaignType_Campaign));
    if(!campaign)
    {
        qDebug() << "[LayerScene] ERROR in postProcessXML: could not find campaign pointer!";
        return;
    }

    QDomElement layerElement = element.firstChildElement(QString("layer"));
    while(!layerElement.isNull())
    {
        QUuid layerID = QUuid(layerElement.attribute(QString("_baseID")));
        if(!layerID.isNull())
        {
            int i = 0;
            while((i < _layers.count()) && (_layers.at(i)->getID() != layerID))
                ++i;

            if((i < _layers.count()) && (_layers.at(i)))
            {
                _layers.at(i)->postProcessXML(campaign, layerElement, isImport);

                if(_layers.at(i)->getType() == DMHelper::LayerType_Reference)
                {
                    LayerReference* referenceLayer = dynamic_cast<LayerReference*>(_layers[i]);
                    if(referenceLayer)
                        connect(referenceLayer, &LayerReference::referenceDestroyed, this, &LayerScene::handleReferenceDestroyed);
                }
            }
        }

        layerElement = layerElement.nextSiblingElement(QString("layer"));
    }

    updateLayerScales();
}

void LayerScene::copyValues(const CampaignObjectBase* other)
{
    const LayerScene* otherScene = dynamic_cast<const LayerScene*>(other);
    if(!otherScene)
        return;

    clearLayers();

    _scale = otherScene->_scale;
    _selected = otherScene->_selected;

    for(int i = 0; i < otherScene->_layers.count(); ++i)
    {
        Layer* newLayer = otherScene->_layers[i]->clone();
        connectLayer(newLayer);
        //connect(newLayer, &Layer::dirty, this, &LayerScene::handleLayerDirty);
        _layers.append(newLayer);
        updateLayerScales();
    }

    CampaignObjectBase::copyValues(other);
}

bool LayerScene::isTreeVisible() const
{
    return false;
}

QGraphicsScene* LayerScene::getDMScene()
{
    return _dmScene;
}

PublishGLScene* LayerScene::getPlayerGLScene()
{
    return _playerGLScene;
}

QRectF LayerScene::boundingRect() const
{
    QRectF result;

    for(int i = 0; i < _layers.count(); ++i)
        result = result.united(_layers.at(i)->boundingRect());

    return result;
}

QSizeF LayerScene::sceneSize() const
{
    return boundingRect().size();
}

int LayerScene::getScale() const
{
    //    LayerGrid* layer = dynamic_cast<LayerGrid*>(_map->getLayerScene().getPriority(DMHelper::LayerType_Grid));
    //    return layer ? layer->getConfig().getGridScale() : DMHelper::STARTING_GRID_SCALE;

    return _scale;
}

void LayerScene::setScale(int scale)
{
    if((scale == _scale) || (scale <= 0))
        return;

    _scale = scale;
    if(_playerGLScene)
        _playerGLScene->setGridScale(scale);

    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->setScale(scale);

    handleLayerDirty();
}

int LayerScene::layerCount() const
{
    return _layers.count();
}

int LayerScene::layerCount(DMHelper::LayerType type) const
{
    int count = 0;

    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type))
            ++count;
    }

    return count;
}

Layer* LayerScene::layerAt(int position) const
{
    return ((position >= 0) && (position < _layers.count())) ? _layers.at(position) : nullptr;
}

void LayerScene::insertLayer(int position, Layer* layer)
{
    if((position < 0) || (position >= _layers.count()) || (!layer) || (_layers.contains(layer)))
    {
        qDebug() << "[LayerScene] ERROR: invalid layer insertion, position: " << position << ", layer: " << layer;
        return;
    }

    if(_initialized)
        layer->initialize(sceneSize().toSize());

    layer->setScale(_scale);
    layer->setLayerScene(this);
    connectLayer(layer);

    if(_dmScene)
        layer->dmInitialize(_dmScene);

    _layers.insert(position, layer);
    resetLayerOrders();
    _selected = position;
    emit layerAdded(layer);
    updateLayerScales();
    handleLayerDirty();
}

void LayerScene::prependLayer(Layer* layer)
{
    if((!layer) || (_layers.contains(layer)))
    {
        qDebug() << "[LayerScene] ERROR: invalid layer prepend, layer: " << layer;
        return;
    }

    if(_initialized)
        layer->initialize(sceneSize().toSize());

    layer->setScale(_scale);
    layer->setLayerScene(this);
    connectLayer(layer);

    if(_dmScene)
        layer->dmInitialize(_dmScene);

    _layers.prepend(layer);
    resetLayerOrders();
    _selected = 0;
    emit layerAdded(layer);
    updateLayerScales();
    handleLayerDirty();
}

void LayerScene::appendLayer(Layer* layer)
{
    if((!layer) || (_layers.contains(layer)))
    {
        qDebug() << "[LayerScene] ERROR: invalid layer append, layer: " << layer;
        return;
    }

    if(_initialized)
        layer->initialize(sceneSize().toSize());

    layer->setScale(_scale);
    layer->setLayerScene(this);
    connectLayer(layer);

    if(_dmScene)
        layer->dmInitialize(_dmScene);

    _layers.append(layer);
    resetLayerOrders();
    _selected = _layers.count() - 1;
    emit layerAdded(layer);
    updateLayerScales();
    handleLayerDirty();
}

void LayerScene::removeLayer(int position)
{
    if((position < 0) || (position >= _layers.count()))
    {
        qDebug() << "[LayerScene] ERROR: invalid layer removal, position: " << position;
        return;
    }

    Layer* deleteLayer = _layers.takeAt(position);
    if(!deleteLayer)
        return;

    disconnectLayer(deleteLayer);
    emit layerRemoved(deleteLayer);
    deleteLayer->deleteLater();

    resetLayerOrders();
    if(_selected >= position)
        --_selected;

    updateLayerScales();
    handleLayerDirty();
}

void LayerScene::clearLayers()
{
    foreach(Layer* layer, _layers)
    {
        if(layer)
            layer->aboutToDelete();
    }

    qDeleteAll(_layers);
    _layers.clear();
    _selected = -1;
}

void LayerScene::moveLayer(int from, int to)
{
    if((from < 0) || (from >= _layers.count()) || (to < 0) || (to >= _layers.count()) || (from == to))
    {
        qDebug() << "[LayerScene] ERROR: invalid layer movement, from: " << from << ", to: " << to;
        return;
    }

    _layers.move(from, to);
    resetLayerOrders();
    _selected = to;
    updateLayerScales();
    handleLayerDirty();
}

Layer* LayerScene::findLayer(QUuid id)
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getID() == id))
            return _layers[i];
    }

    return nullptr;
}

int LayerScene::getSelectedLayerIndex() const
{
    return _selected;
}

void LayerScene::setSelectedLayerIndex(int selected)
{
    if(_selected == selected)
        return;

    if((selected < 0) || (selected >= _layers.count()))
    {
        qDebug() << "[LayerScene] ERROR: invalid layer selection, current selected: " << _selected << ", new selected: " << selected << ", count: " << _layers.count();
        return;
    }

    _selected = selected;
    emit layerSelected(getSelectedLayer());
    handleLayerDirty();
}

Layer* LayerScene::getSelectedLayer() const
{
    if((_selected >= 0) && (_selected < _layers.count()))
        return _layers.at(_selected);
    else
        return nullptr;
}

void LayerScene::setSelectedLayer(Layer* layer)
{
    setSelectedLayerIndex(getLayerIndex(layer));
}

int LayerScene::getLayerIndex(Layer* layer) const
{
    if(layer)
    {
        for(int i = 0; i < _layers.count(); ++i)
        {
            if((_layers.at(i)) && (_layers.at(i) == layer))
                return i;
        }
    }

    return -1;
}

Layer* LayerScene::getPriority(DMHelper::LayerType type) const
{
    Layer* result = getSelectedLayer();
    if((result) && (result->getFinalType() == type))
        return result->getFinalLayer();
    else
        return getFirstVisible(type, true);
}

Layer* LayerScene::getFirst(DMHelper::LayerType type) const
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type))
            return _layers.at(i)->getFinalLayer();
    }

    return nullptr;
}

Layer* LayerScene::getFirstVisible(DMHelper::LayerType type, bool dmVisible) const
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type) && (_layers.at(i)->getLayerVisibleDM() == dmVisible))
            return _layers.at(i)->getFinalLayer();
    }

    return nullptr;
}

Layer* LayerScene::getPrevious(Layer* layer, DMHelper::LayerType type) const
{
    if(!layer)
        return getPriority(type);

    int latest = -1;
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers.at(i))
        {
            if(_layers.at(i) == layer)
                return latest >= 0 ? _layers.at(latest)->getFinalLayer() : nullptr;

            if(_layers.at(i)->getFinalType() == type)
                latest = i;
        }
    }

    return nullptr;
}

Layer* LayerScene::getNext(Layer* layer, DMHelper::LayerType type) const
{
    if(!layer)
        return getPriority(type);

    for(int i = getLayerIndex(layer); i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type))
            return _layers.at(i)->getFinalLayer();
    }

    return nullptr;
}

Layer* LayerScene::getNearest(Layer* layer, DMHelper::LayerType type) const
{
    if(!layer)
        return getFirst(type);

    if(layer->getFinalType() == type)
        return layer->getFinalLayer();

    Layer* previousLayer = getPrevious(layer, type);
    return previousLayer ? previousLayer : getNext(layer, type);
}

QImage LayerScene::mergedImage()
{
    QImage result(sceneSize().toSize(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter;

    painter.begin(&result);

    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers.at(i))
        {
            QImage oneImage = getLayerImage(_layers.at(i));
            if(!oneImage.isNull())
                painter.drawImage(_layers.at(i)->getPosition(), oneImage);
        }
    }

    painter.end();

    return result;
}

PublishGLRenderer* LayerScene::getRenderer() const
{
    return _renderer;
}

QList<Layer*> LayerScene::getLayers() const
{
    return _layers;
}

QList<Layer*> LayerScene::getLayers(DMHelper::LayerType type) const
{
    QList<Layer*> result;

    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type))
            result.append(_layers.at(i));
    }

    return result;
}

void LayerScene::initializeLayers()
{
    if(_initialized)
        return;

    // First initialize images to find the size of the scene
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers[i]->getFinalType() == DMHelper::LayerType_Image)
            _layers[i]->initialize(QSize());
    }

    QSize currentSize = sceneSize().toSize();

    // Initialize other layers, telling them how big they should be
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers[i]->getFinalType() != DMHelper::LayerType_Image)
            _layers[i]->initialize(currentSize);
    }

    _initialized = true;
}

void LayerScene::uninitializeLayers()
{
    if(!_initialized)
        return;

    dmUninitialize();
    playerGLUninitialize();

    for(int i = 0; i < _layers.count(); ++i)
    {
        _layers[i]->uninitialize();
    }

    _initialized = false;
}

void LayerScene::dmInitialize(QGraphicsScene* scene)
{
    _dmScene = scene;

    initializeLayers();
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmInitialize(scene);
}

void LayerScene::dmUninitialize()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmUninitialize();

    _dmScene = nullptr;
}

void LayerScene::dmUpdate()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmUpdate();
}

void LayerScene::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    initializeLayers();

    _renderer = renderer;

    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLInitialize(renderer, scene);

    _playerGLScene = scene;
}

void LayerScene::playerGLUninitialize()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLUninitialize();

    _renderer = nullptr;
    _playerGLScene = nullptr;
}

bool LayerScene::playerGLUpdate()
{
    bool result = false;

    for(int i = 0; i < _layers.count(); ++i)
        result = result || _layers[i]->playerGLUpdate();

    return result;
}

void LayerScene::playerGLPaint(QOpenGLFunctions* functions, unsigned int shaderProgram, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    if(!functions)
        return;

    if((shaderProgram == 0) || (defaultModelMatrix == -1))
    {
        qDebug() << "[LayerScene] playerGLPaint ERROR: invalid shaders program or matrix! shaderProgram: " << shaderProgram << ", defaultModelMatrix: " << defaultModelMatrix;
        return;
    }

    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getLayerVisiblePlayer()) && (_layers.at(i)->getOpacity() > 0.0))
        {
            if(!_layers.at(i)->playerIsInitialized())
                _layers[i]->playerGLInitialize(_renderer, _playerGLScene);

            if(_layers.at(i)->playerIsInitialized())
            {
                if(_layers.at(i)->defaultShader())
                {
#ifdef DEBUG_LAYERSCENE
                    qDebug() << "[LayerScene]::playerGLPaint UseProgram: " << shaderProgram;
#endif
                    functions->glUseProgram(shaderProgram);
                }

                _layers[i]->playerGLPaint(functions, defaultModelMatrix, projectionMatrix);
            }
        }
    }
}

void LayerScene::playerGLResize(int w, int h)
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLResize(w, h);
}

void LayerScene::playerSetShaders(unsigned int programRGB, int modelMatrixRGB, int projectionMatrixRGB, unsigned int programRGBA, int modelMatrixRGBA, int projectionMatrixRGBA, int alphaRGBA)
{
#ifdef DEBUG_LAYERSCENE
    qDebug() << "[LayerScene] playerSetShaders: programRGB: " << programRGB << ", modelMatrixRGB: " << modelMatrixRGB << ", projectionMatrixRGB: " << projectionMatrixRGB << ", programRGBA: " << programRGBA << ", modelMatrixRGBA: " << modelMatrixRGBA << ", projectionMatrixRGBA: " << projectionMatrixRGBA << ", alphaRGBA: " << alphaRGBA;
#endif

    if((programRGB == 0) || (programRGBA == 0))
    {
        qDebug() << "[LayerScene] ERROR: invalid shaders set! programRGB: " << programRGB << ", modelMatrixRGB: " << modelMatrixRGB << ", projectionMatrixRGB: " << projectionMatrixRGB << ", programRGBA: " << programRGBA << ", modelMatrixRGBA: " << modelMatrixRGBA << ", projectionMatrixRGBA: " << projectionMatrixRGBA << ", alphaRGBA: " << alphaRGBA;
        return;
    }

    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerSetShaders(programRGB, modelMatrixRGB, projectionMatrixRGB, programRGBA, modelMatrixRGBA, projectionMatrixRGBA, alphaRGBA);
}

void LayerScene::handleLayerDirty()
{
    if(_initialized)
        emit dirty();
}

void LayerScene::handleLayerScaleChanged(Layer* layer)
{
    updateLayerScales();
    emit layerScaleChanged(layer);
    handleLayerDirty();
}

void LayerScene::updateLayerScales()
{
    foreach(Layer* singleLayer, _layers)
    {
        LayerGrid* gridLayer = dynamic_cast<LayerGrid*>(getNearest(singleLayer, DMHelper::LayerType_Grid));
        singleLayer->setScale(gridLayer ? gridLayer->getConfig().getGridScale() : _scale);
    }
}

void LayerScene::removeLayer(Layer* reference)
{
    if(!reference)
        return;

    removeLayer(reference->getOrder());
}

void LayerScene::layerMoved(const QPoint& position)
{
    Q_UNUSED(position);
    emit sceneChanged();
}

void LayerScene::layerResized(const QSize& size)
{
    Q_UNUSED(size);

    QSize currentSize = sceneSize().toSize();

    // Check if there is a null-sized FoW layer
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers[i]->getFinalType() == DMHelper::LayerType_Fow) && (_layers[i]->getSize().isEmpty()))
            _layers[i]->setSize(currentSize);
    }

    emit sceneChanged();
    emit sceneSizeChanged();
}

void LayerScene::handleReferenceDestroyed(Layer* source, Layer* reference)
{
    foreach(Layer* layer, _layers)
    {
        if((layer) && (layer != source) && (layer->getType() == DMHelper::LayerType_Reference))
        {
            LayerReference* layerReference = dynamic_cast<LayerReference*>(layer);
            if(layerReference->getReferenceLayer() == reference)
                layerReference->clearReference();
        }
    }

    removeLayer(source);
}

QDomElement LayerScene::createOutputXML(QDomDocument &doc)
{
    if(_layers.count() > 0)
        return doc.createElement("layer-scene");
    else
        return QDomElement();
}

void LayerScene::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("selected", _selected);
    element.setAttribute("scale", _scale);
    // TODO: Layers - store the scene rect as well

    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->outputXML(doc, element, targetDirectory, isExport);
}

void LayerScene::connectLayer(Layer* layer)
{
    if(!layer)
        return;

    connect(layer, &Layer::dirty, this, &LayerScene::handleLayerDirty);
    connect(layer, &Layer::layerMoved, this, &LayerScene::sceneChanged);
    connect(layer, &Layer::layerResized, this, &LayerScene::layerResized);
    connect(layer, &Layer::layerResized, this, &LayerScene::sceneChanged);
    connect(layer, &Layer::layerVisibilityChanged, this, &LayerScene::layerVisibilityChanged);
    connect(layer, &Layer::layerScaleChanged, this, &LayerScene::handleLayerScaleChanged);
}

void LayerScene::disconnectLayer(Layer* layer)
{
    if(!layer)
        return;

    disconnect(layer, &Layer::dirty, this, &LayerScene::handleLayerDirty);
    disconnect(layer, &Layer::layerMoved, this, &LayerScene::sceneChanged);
    disconnect(layer, &Layer::layerResized, this, &LayerScene::layerResized);
    disconnect(layer, &Layer::layerResized, this, &LayerScene::sceneChanged);
    disconnect(layer, &Layer::layerVisibilityChanged, this, &LayerScene::layerVisibilityChanged);
    disconnect(layer, &Layer::layerScaleChanged, this, &LayerScene::handleLayerScaleChanged);
}

void LayerScene::resetLayerOrders()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->setOrder(i);
}

QImage LayerScene::getLayerImage(Layer* layer)
{
    if(!layer)
        return QImage();

    if(layer->getType() == DMHelper::LayerType_Image)
    {
        LayerImage* layerImage = dynamic_cast<LayerImage*>(layer);
        if(layerImage)
            return layerImage->getImage();
    }
    else if(layer->getType() == DMHelper::LayerType_Video)
    {
        LayerVideo* layerVideo = dynamic_cast<LayerVideo*>(layer);
        if(layerVideo)
            return layerVideo->getScreenshot();
    }
    else if(layer->getType() == DMHelper::LayerType_Fow)
    {
        LayerFow* layerFow = dynamic_cast<LayerFow*>(layer);
        if(layerFow)
            return layerFow->getImage();
    }
    else if(layer->getType() == DMHelper::LayerType_Blank)
    {
        LayerBlank* layerBlank = dynamic_cast<LayerBlank*>(layer);
        if(layerBlank)
            return layerBlank->getImage();
    }
    else if(layer->getType() == DMHelper::LayerType_Reference)
    {
        LayerReference* layerReference = dynamic_cast<LayerReference*>(layer);
        if(layerReference)
            return getLayerImage(layerReference->getReferenceLayer());
    }

    return QImage();
}
