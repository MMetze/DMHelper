#include "publishglrenderer.h"
#include "campaignobjectbase.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QUuid>
#include <QDebug>

PublishGLRenderer::PublishGLRenderer(QObject *parent) :
    QObject(parent),
    _targetWidget(nullptr),
    _rotation(0)
{
}

PublishGLRenderer::~PublishGLRenderer()
{
}

CampaignObjectBase* PublishGLRenderer::getObject()
{
    return nullptr;
}

QUuid PublishGLRenderer::getObjectId()
{
    CampaignObjectBase* obj = getObject();
    if(obj)
        return obj->getID();
    else
        return QUuid();
}

QColor PublishGLRenderer::getBackgroundColor()
{
    return QColor();
}

void PublishGLRenderer::rendererActivated(QOpenGLWidget* glWidget)
{
    _targetWidget = glWidget;

    if((_targetWidget) && (_targetWidget->context()))
        connect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanup);
}

void PublishGLRenderer::rendererDeactivated()
{
    qDebug() << "[PublishGLRenderer] Renderer deactivated: " << this;
    cleanup();

    if((_targetWidget) && (_targetWidget->context()))
        disconnect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanup);

    emit deactivated();
    _targetWidget = nullptr;
}

bool PublishGLRenderer::deleteOnDeactivation()
{
    return false;
}

void PublishGLRenderer::updateRender()
{
    emit updateWidget();
}

void PublishGLRenderer::setBackgroundColor(const QColor& color)
{
    Q_UNUSED(color);
}

void PublishGLRenderer::setRotation(int rotation)
{
    if(rotation != _rotation)
    {
        _rotation = rotation;
        updateProjectionMatrix();
        emit updateWidget();
    }
}

void PublishGLRenderer::updateProjectionMatrix()
{
}

