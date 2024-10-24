#include "publishglrenderer.h"
#include "publishglimage.h"
#include "campaignobjectbase.h"
#include "dmconstants.h"
#include "dmh_opengl.h"
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QUuid>
#include <QDebug>

PublishGLRenderer::PublishGLRenderer(QObject *parent) :
    QObject(parent),
    _targetWidget(nullptr),
    _rotation(0),
    _pointerImage(nullptr),
    _pointerActive(false),
    _pointerPos(),
    _pointerFile()
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
        connect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanupGL);
}

void PublishGLRenderer::rendererDeactivated()
{
    qDebug() << "[PublishGLRenderer] Renderer deactivated: " << this;
    if((_targetWidget) && (_targetWidget->context()))
        disconnect(_targetWidget->context(), &QOpenGLContext::aboutToBeDestroyed, this, &PublishGLRenderer::cleanupGL);

    emit deactivated();
    _targetWidget = nullptr;
}

bool PublishGLRenderer::deleteOnDeactivation()
{
    return false;
}

QRect PublishGLRenderer::getScissorRect()
{
    return QRect();
}

QOpenGLWidget* PublishGLRenderer::getTargetWidget()
{
    return _targetWidget;
}

void PublishGLRenderer::updateRender()
{
    emit updateWidget();
}

void PublishGLRenderer::cleanupGL()
{
    delete _pointerImage;
    _pointerImage = nullptr;
}

void PublishGLRenderer::setBackgroundColor(const QColor& color)
{
    Q_UNUSED(color);
}

void PublishGLRenderer::setRotation(int rotation)
{
    if(rotation == _rotation)
        return;

    _rotation = rotation;
    updateProjectionMatrix();
    emit updateWidget();
}

void PublishGLRenderer::pointerToggled(bool enabled)
{
    if(_pointerActive == enabled)
        return;

    _pointerActive = enabled;
    emit updateWidget();
}

void PublishGLRenderer::setPointerPosition(const QPointF& pos)
{
    if(_pointerPos == pos)
        return;

    _pointerPos = pos;
    emit updateWidget();
}

void PublishGLRenderer::setPointerFileName(const QString& filename)
{
    if(_pointerFile == filename)
        return;

    _pointerFile = filename;
    delete _pointerImage;
    _pointerImage = nullptr;
    emit updateWidget();
}

void PublishGLRenderer::paintPointer(QOpenGLFunctions* functions, const QSize& sceneSize, int shaderModelMatrix)
{
    if((!_pointerImage) || (!functions))
        return;

    QPointF pointPos(_pointerPos.x() - (sceneSize.width() / 2.0) - (DMHelper::CURSOR_SIZE / 2), (sceneSize.height() / 2.0) - _pointerPos.y() + (DMHelper::CURSOR_SIZE / 2) - _pointerImage->getSize().height());
    _pointerImage->setPosition(pointPos);
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, _pointerImage->getMatrixData(), _pointerImage->getMatrix());
    functions->glUniformMatrix4fv(shaderModelMatrix, 1, GL_FALSE, _pointerImage->getMatrixData());
    _pointerImage->paintGL(functions, nullptr);
}

void PublishGLRenderer::setPointerScale(qreal pointerScale)
{
    if(!_pointerImage)
        return;

    _pointerImage->setScale(pointerScale);
}

void PublishGLRenderer::evaluatePointer()
{
    if(_pointerActive)
    {
        if(!_pointerImage)
            _pointerImage = new PublishGLImage(getPointerPixmap().scaled(DMHelper::CURSOR_SIZE * 2, DMHelper::CURSOR_SIZE * 2, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).toImage(), false);
    }
    else if(_pointerImage)
    {
        delete _pointerImage;
        _pointerImage = nullptr;
    }
}

QPixmap PublishGLRenderer::getPointerPixmap()
{
    if(!_pointerFile.isEmpty())
    {
        QPixmap result;
        if(result.load(_pointerFile))
            return result;
    }

    return QPixmap(":/img/data/arrow.png");
}

