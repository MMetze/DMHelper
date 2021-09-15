#include "publishglrenderer.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>

PublishGLRenderer::PublishGLRenderer(QObject *parent) :
    QObject(parent),
    _targetWidget(nullptr)
{
}

PublishGLRenderer::~PublishGLRenderer()
{
}

void PublishGLRenderer::rendererActivated(QOpenGLWidget* glWidget)
{
    _targetWidget = glWidget;

    if((_targetWidget) && (_targetWidget->context()))
        connect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanup);
}

void PublishGLRenderer::rendererDeactivated()
{
    if((_targetWidget) && (_targetWidget->context()))
        disconnect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanup);

    _targetWidget = nullptr;
}

bool PublishGLRenderer::deleteOnDeactivation()
{
    return false;
}
