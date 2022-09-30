#include "layerscene.h"
#include "layer.h"
#include "layerimage.h"
#include "layerfow.h"
#include <QRectF>
#include <QImage>
#include <QPainter>
#include <QDomElement>

LayerScene::LayerScene(QObject *parent) :
    CampaignObjectBase{QString(), parent},
    _initialized(false),
    _layers(),
    _selected(-1),
    _dmScene(nullptr)
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

    QDomElement layerElement = element.firstChildElement(QString("layer"));
    while(!layerElement.isNull())
    {
        Layer* newLayer = nullptr;
        switch( layerElement.attribute(QString("type")).toInt())
        {
            case DMHelper::LayerType_Image:
                newLayer = new LayerImage();
                break;
            case DMHelper::LayerType_Fow:
                newLayer = new LayerFow();
                break;
            default:
                break;
        }

        if(newLayer)
        {
            newLayer->inputXML(layerElement, isImport);
            _layers.append(newLayer);
        }

        layerElement = layerElement.nextSiblingElement(QString("layer"));
    }
}

void LayerScene::copyValues(const CampaignObjectBase* other)
{
    const LayerScene* otherScene = dynamic_cast<const LayerScene*>(other);
    if(!otherScene)
        return;

    clearLayers();

    _selected = otherScene->_selected;

    for(int i = 0; i < otherScene->_layers.count(); ++i)
        _layers.append(otherScene->_layers[i]->clone());

    CampaignObjectBase::copyValues(other);
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

    if(_dmScene)
        layer->dmInitialize(*_dmScene);

    _layers.insert(position, layer);
    resetLayerOrders();
    _selected = position;
}

void LayerScene::prependLayer(Layer* layer)
{
    if(!layer)
        return;

    if(_dmScene)
        layer->dmInitialize(*_dmScene);

    _layers.prepend(layer);
    resetLayerOrders();
    _selected = 0;
}

void LayerScene::appendLayer(Layer* layer)
{
    if(!layer)
        return;

    if(_dmScene)
        layer->dmInitialize(*_dmScene);

    _layers.append(layer);
    resetLayerOrders();
    _selected = _layers.count() - 1;
}

void LayerScene::removeLayer(int position)
{
    if((position < 0) || (position >= _layers.count()))
        return;

    Layer* deleteLayer = _layers.takeAt(position);
    if(deleteLayer)
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
    if((result) && (result->getType() == type))
        return result;
    else
        return getFirst(type);
}

Layer* LayerScene::getFirst(DMHelper::LayerType type) const
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getType() == type))
            return _layers.at(i);
    }

    return nullptr;
}

QImage LayerScene::mergedImage()
{
    QImage result;

    // TODO: add different layer types here
    for(int i = 0; i < _layers.count(); ++i)
    {
        if((_layers.at(i)) && (_layers.at(i)->getType() == DMHelper::LayerType_Image))
        {
            LayerImage* layerImage = dynamic_cast<LayerImage*>(_layers.at(i));
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

void LayerScene::initializeLayers()
{
    // First initialize images to find the size of the scene
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers[i]->getType() == DMHelper::LayerType_Image)
            _layers[i]->initialize();
    }

    QSizeF currentSize = sceneSize();

    // Initialize other layers, telling them how big they should be
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers[i]->getType() != DMHelper::LayerType_Image)
        {
            _layers[i]->initialize();
        }
    }
}

void LayerScene::uninitializeLayers()
{

}

void LayerScene::dmInitialize(QGraphicsScene& scene)
{
    initializeLayers();
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmInitialize(scene);

    _dmScene = &scene;
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

void LayerScene::playerGLInitialize()
{
    initializeLayers();
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLInitialize();
}

void LayerScene::playerGLUninitialize()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLUninitialize();
}

bool LayerScene::playerGLUpdate()
{
    bool result = false;

    for(int i = 0; i < _layers.count(); ++i)
        result = result || _layers[i]->playerGLUpdate();

    return result;
}

void LayerScene::playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix)
{
    for(int i = 0; i < _layers.count(); ++i)
    {
        if(_layers.at(i)->getLayerVisible())
            _layers[i]->playerGLPaint(functions, modelMatrix);
    }
}

void LayerScene::playerGLResize(int w, int h)
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLResize(w, h);
}

QDomElement LayerScene::createOutputXML(QDomDocument &doc)
{
    return doc.createElement("layerScene");
}

void LayerScene::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("selected", _selected);

    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->outputXML(doc, element, targetDirectory, isExport);
}

void LayerScene::resetLayerOrders()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->setOrder(i);
}
