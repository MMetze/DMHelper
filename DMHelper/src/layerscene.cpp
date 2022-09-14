#include "layerscene.h"
#include "layer.h"
#include "layerimage.h"
#include <QRectF>
#include <QImage>
#include <QPainter>

LayerScene::LayerScene(QObject *parent) :
    QObject{parent},
    _layers(),
    _selected(-1)
{
}

LayerScene::~LayerScene()
{
    qDeleteAll(_layers);
}

QRectF LayerScene::boundingRect() const
{
    QRectF result;

    for(int i = 0; i < _layers.count(); ++i)
    {
        result = result.united(_layers.at(i)->boundingRect());
    }

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

    _layers.insert(position, layer);
    resetLayerOrders();
    _selected = position;
}

void LayerScene::prependLayer(Layer* layer)
{
    if(!layer)
        return;

    _layers.prepend(layer);
    resetLayerOrders();
    _selected = 0;
}

void LayerScene::appendLayer(Layer* layer)
{
    if(!layer)
        return;

    _layers.append(layer);
    resetLayerOrders();
    _selected = _layers.count() - 1;
}

void LayerScene::removeLayer(int position)
{
    if((position < 0) || (position >= _layers.count()))
        return;

    _layers.removeAt(position);
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

void LayerScene::dmInitialize(QGraphicsScene& scene)
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmInitialize(scene);
}

void LayerScene::dmUninitialize()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmUninitialize();
}

void LayerScene::dmUpdate()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->dmUpdate();
}

void LayerScene::playerGLInitialize()
{
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

void LayerScene::resetLayerOrders()
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->setOrder(i);
}
