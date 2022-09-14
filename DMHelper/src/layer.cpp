#include "layer.h"
#include <QRectF>
#include <QImage>

Layer::Layer(const QString& name, int order, QObject *parent) :
    QObject{parent},
    _name(name),
    _order(order),
    _layerVisible(true)
{
}

Layer::~Layer()
{
}

QRectF Layer::boundingRect() const
{
    return QRectF();
}

QString Layer::getName() const
{
    return _name;
}

int Layer::getOrder() const
{
    return _order;
}

bool Layer::getLayerVisible() const
{
    return _layerVisible;
}

QImage Layer::getLayerIcon() const
{
    return QImage();
}

bool Layer::playerGLUpdate()
{
    return false;
}

void Layer::setName(const QString& name)
{
    if(_name == name)
        return;

    _name = name;
    emit dirty();
}

void Layer::setOrder(int order)
{
    if(_order == order)
        return;

    _order = order;
    emit dirty();
    emit orderChanged(order);
}

void Layer::setLayerVisible(bool layerVisible)
{
    if(_layerVisible == layerVisible)
        return;

    _layerVisible = layerVisible;
    emit dirty();
}
