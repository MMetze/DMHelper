#include "layerfow.h"
#include <QImage>

LayerFow::LayerFow(int order, QObject *parent) :
    Layer{order, parent}
{
}

LayerFow::~LayerFow()
{
}

QRectF LayerFow::boundingRect() const
{
    return QRectF();
    //return _graphicsItem ? _graphicsItem->boundingRect() : QRectF();
}

DMHelper::LayerType LayerFow::getType() const
{
    return DMHelper::LayerType_Fow;
}

QImage LayerFow::getImage() const
{
    return QImage();
}

void LayerFow::dmInitialize(QGraphicsScene& scene)
{
}

void LayerFow::dmUninitialize()
{
}

void LayerFow::dmUpdate()
{
}

void LayerFow::playerGLInitialize()
{
}

void LayerFow::playerGLUninitialize()
{
}

/*
bool LayerFow::playerGLUpdate()
{
}
*/

void LayerFow::playerGLPaint(QOpenGLFunctions* functions, GLint modelMatrix)
{
}

void LayerFow::playerGLResize(int w, int h)
{
}
