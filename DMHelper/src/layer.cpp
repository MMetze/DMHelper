#include "layer.h"
#include <QRectF>

Layer::Layer(int order, QObject *parent) :
    QObject{parent},
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

int Layer::getOrder() const
{
    return _order;
}

void Layer::setOrder(int order)
{
    if(_order == order)
        return;

    _order = order;
    emit orderChanged(order);
}

bool Layer::playerGLUpdate()
{
    return false;
}
