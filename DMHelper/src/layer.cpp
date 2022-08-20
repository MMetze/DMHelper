#include "layer.h"
#include <QRectF>

Layer::Layer(QObject *parent) :
    QObject{parent}
{
}

Layer::~Layer()
{
}

QRectF Layer::boundingRect() const
{
    return QRectF();
}
