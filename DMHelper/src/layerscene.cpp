#include "layerscene.h"
#include "layer.h"
#include <QRectF>

LayerScene::LayerScene(QObject *parent) :
    QObject{parent},
    _layers()
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

int LayerScene::layerCount()
{
    return _layers.count();
}

Layer* LayerScene::layerAt(int position)
{
    return ((position >= 0) && (position < _layers.count())) ? _layers.at(position) : nullptr;
}

void LayerScene::insertLayer(int position, Layer* layer)
{
    if((position < 0) || (position >= _layers.count()) || (!layer))
        return;

    _layers.insert(position, layer);
}

void LayerScene::prependLayer(Layer* layer)
{
    if(!layer)
        return;

    _layers.prepend(layer);
}

void LayerScene::appendLayer(Layer* layer)
{
    if(!layer)
        return;

    _layers.append(layer);
}

void LayerScene::removeLayer(int position)
{
    if((position < 0) || (position >= _layers.count()))
        return;

    _layers.removeAt(position);
}

void LayerScene::clearLayers()
{
    qDeleteAll(_layers);
    _layers.clear();
}

void LayerScene::moveLayer(int from, int to)
{
    if((from < 0) || (from >= _layers.count()) || (to < 0) || (to >= _layers.count()))
        return;

    _layers.move(from, to);
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
        _layers[i]->playerGLPaint(functions, modelMatrix);
}

void LayerScene::playerGLResize(int w, int h)
{
    for(int i = 0; i < _layers.count(); ++i)
        _layers[i]->playerGLResize(w, h);
}
