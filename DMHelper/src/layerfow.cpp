#include "layerfow.h"

LayerFow::LayerFow(int order, QObject *parent)
    : Layer{order, parent}
{
}

LayerFow::~LayerFow()
{
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
