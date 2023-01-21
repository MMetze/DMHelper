    #include "layerscene.h"
#include "layer.h"
#include "layerimage.h"
#include "layerfow.h"
#include "layergrid.h"
#include "layerreference.h"
#include "campaign.h"
#include <QRectF>
#include <QImage>
#include <QPainter>
#include <QDomElement>
#include <QDebug>

LayerScene::LayerScene(QObject *parent) :
    CampaignObjectBase{QString(), parent},
    _initialized(false),
    _layers(),
    _scale(DMHelper::STARTING_GRID_SCALE),
    _selected(-1),
    _dmScene(nullptr),
    _playerGLScene(nullptr)
{
}

LayerScene::~LayerScene()
{
    qDeleteAll(_layers);
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
        switch(layerElement.attribute(QString("type")).toInt())
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
            default:
                break;
        }

        if(newLayer)
        {
            newLayer->inputXML(layerElement, isImport);
            newLayer->setScale(_scale);
            newLayer->setLayerScene(this);
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

    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers.at(i))
        {
            _layers.at(i)->postProcessXML(campaign, element, isImport);

            if(_layers.at(i)->getType() == DMHelper::LayerType_Reference)
            {
                LayerReference* referenceLayer = dynamic_cast<LayerReference*>(_layers[i]);
                if(referenceLayer)
                    connect(referenceLayer, &LayerReference::referenceDestroyed, this, QOverload<Layer*>::of(&LayerScene::removeLayer));
            }
        }
    }
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
        _layers.append(otherScene->_layers[i]->clone());

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
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->setScale(scale);
}

int LayerScene::layerCount() const
{
    return _layers.count();
}

Layer* LayerScene::layerAt(int position) const
{
    return ((position >= 0) && (position < _layers.count())) ? _layers.at(position) : nullptr;
}

void LayerScene::insertLayer(int position, Layer* layer)
{
    if((position < 0) || (position >= _layers.count()) || (!layer))
        return;

    if(_initialized)
        layer->initialize(sceneSize().toSize());

    layer->setScale(_scale);
    layer->setLayerScene(this);

    if(_dmScene)
        layer->dmInitialize(_dmScene);

    _layers.insert(position, layer);
    resetLayerOrders();
    _selected = position;
    emit layerAdded(layer);
}

void LayerScene::prependLayer(Layer* layer)
{
    if(!layer)
        return;

    if(_initialized)
        layer->initialize(sceneSize().toSize());

    layer->setScale(_scale);
    layer->setLayerScene(this);

    if(_dmScene)
        layer->dmInitialize(_dmScene);

    _layers.prepend(layer);
    resetLayerOrders();
    _selected = 0;
    emit layerAdded(layer);
}

void LayerScene::appendLayer(Layer* layer)
{
    if(!layer)
        return;

    if(_initialized)
        layer->initialize(sceneSize().toSize());

    layer->setScale(_scale);
    layer->setLayerScene(this);

    if(_dmScene)
        layer->dmInitialize(_dmScene);

    _layers.append(layer);
    resetLayerOrders();
    _selected = _layers.count() - 1;
    emit layerAdded(layer);
}

void LayerScene::removeLayer(int position)
{
    if((position < 0) || (position >= _layers.count()))
        return;

    Layer* deleteLayer = _layers.takeAt(position);
    if(!deleteLayer)
        return;

    emit layerRemoved(deleteLayer);
    deleteLayer->deleteLater();

    resetLayerOrders();
    if(_selected >= position)
        --_selected;
}

void LayerScene::clearLayers()
{
    qDeleteAll(_layers);
    _layers.clear();
    _selected = -1;
}

void LayerScene::moveLayer(int from, int to)
{
    if((from < 0) || (from >= _layers.count()) || (to < 0) || (to >= _layers.count()))
        return;

    _layers.move(from, to);
    resetLayerOrders();
    _selected = to;
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
    _selected = selected;
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
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i) == layer))
        {
            _selected = i;
            return;
        }
    }
}

Layer* LayerScene::getPriority(DMHelper::LayerType type) const
{
    Layer* result = getSelectedLayer();
    if((result) && (result->getFinalType() == type))
        return result;
    else
        return getFirst(type);
}

int LayerScene::getPriorityIndex(DMHelper::LayerType type) const
{
    Layer* result = getSelectedLayer();
    if((result) && (result->getFinalType() == type))
        return result->getOrder();
    else
        return getFirstIndex(type);
}

Layer* LayerScene::getFirst(DMHelper::LayerType type) const
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type))
            return _layers.at(i);
    }

    return nullptr;
}

int LayerScene::getFirstIndex(DMHelper::LayerType type) const
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == type))
            return i;
    }

    return -1;
}

QImage LayerScene::mergedImage()
{
    QImage result;

    // TODO: add different layer types here
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getFinalType() == DMHelper::LayerType_Image))
        {
            LayerImage* layerImage = nullptr;
            if(_layers.at(i)->getType() == DMHelper::LayerType_Image)
            {
                layerImage = dynamic_cast<LayerImage*>(_layers.at(i));
            }
            else if(_layers.at(i)->getType() == DMHelper::LayerType_Reference)
            {
                LayerReference* layerReference = dynamic_cast<LayerReference*>(_layers.at(i));
                if(layerReference)
                    layerImage = dynamic_cast<LayerImage*>(layerReference->getReferenceLayer());
            }

            if(layerImage)
            {
                if(result.isNull())
                {
                    result = layerImage->getImage();
                }
                else
                {
                    QPainter painter(&result);
                    painter.drawImage(0, 0, layerImage->getImage());
                }
            }
        }
    }

    return result;
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

    QSize currentSize;

    // First initialize images to find the size of the scene
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers[i]->getFinalType() == DMHelper::LayerType_Image)
            _layers[i]->initialize(currentSize);
    }

    currentSize = sceneSize().toSize();

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
    initializeLayers();
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmInitialize(scene);

    _dmScene = scene;
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

void LayerScene::playerGLInitialize(PublishGLScene* scene)
{
    initializeLayers();
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLInitialize(scene);

    _playerGLScene = scene;
}

void LayerScene::playerGLUninitialize()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLUninitialize();

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

    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers.at(i)->getLayerVisible())
        {
            if(_layers.at(i)->defaultShader())
            {
                qDebug() << "[LayerScene]::playerGLPaint UseProgram: " << shaderProgram;
                functions->glUseProgram(shaderProgram);
            }

            _layers[i]->playerGLPaint(functions, defaultModelMatrix, projectionMatrix);
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
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerSetShaders(programRGB, modelMatrixRGB, projectionMatrixRGB, programRGBA, modelMatrixRGBA, projectionMatrixRGBA, alphaRGBA);
}

void LayerScene::removeLayer(Layer* reference)
{
    if(!reference)
        return;

    removeLayer(reference->getOrder());
}

QDomElement LayerScene::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("layer-scene");
}

void LayerScene::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("selected", _selected);
    element.setAttribute("scale", _scale);

    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->outputXML(doc, element, targetDirectory, isExport);
}

void LayerScene::resetLayerOrders()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->setOrder(i);
}
