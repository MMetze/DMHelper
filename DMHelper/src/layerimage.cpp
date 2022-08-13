#include "layerimage.h"
#include <QGraphicsPixmapItem>

LayerImage::LayerImage(QObject *parent) :
    Layer{parent},
    _backgroundImage(nullptr)
{
}

LayerImage::~LayerImage()
{
}

void LayerImage::dmInitialize()
{
}

void LayerImage::dmUninitialize()
{
}

void LayerImage::dmUpdate()
{
}

void LayerImage::playerGLInitialize()
{
}

void LayerImage::playerGLUninitialize()
{
}

void LayerImage::playerGLUpdate()
{
}

void LayerImage::playerGLPaint()
{
}
