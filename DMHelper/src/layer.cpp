#include "layer.h"
#include <QRectF>
#include <QImage>

Layer::Layer(const QString& name, int order, QObject *parent) :
    QObject{parent},
    _name(name),
    _order(order)
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

void Layer::setName(const QString& name)
{
    if(_name == name)
        return;

    _name = name;
    emit dirty();
}

int Layer::getOrder() const
{
    return _order;
}

void Layer::setOrder(int order)
{
    if(_order == order)
        return;

    _order = order;
    emit dirty();
    emit orderChanged(order);
}

QImage Layer::getLayerIcon() const
{
    return QImage();
}

bool Layer::playerGLUpdate()
{
    return false;
}
