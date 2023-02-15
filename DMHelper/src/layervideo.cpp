#include "layervideo.h"
#include "videoplayerglplayer.h"
#include "videoplayerglscreenshot.h"
#include "publishglrenderer.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLWidget>

LayerVideo::LayerVideo(const QString& name, const QString& filename, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
    _videoPlayer(nullptr),
    _filename(filename),
    _layerScreenshot(),
    _dmScene(nullptr)
{
}

LayerVideo::~LayerVideo()
{
    cleanupDM();
    cleanupPlayer();
}

void LayerVideo::inputXML(const QDomElement &element, bool isImport)
{
    // todo
}

QRectF LayerVideo::boundingRect() const
{
    return _layerScreenshot.isNull() ? QRectF() : QRectF(_layerScreenshot.rect());
}

QImage LayerVideo::getLayerIcon() const
{
    return _layerScreenshot.isNull() ? QImage(":/img/data/icon_play.png") : _layerScreenshot;
}

DMHelper::LayerType LayerVideo::getType() const
{
    return DMHelper::LayerType_Video;
}

Layer* LayerVideo::clone() const
{
    LayerVideo* newLayer = new LayerVideo(_name, _filename, _order);

    copyBaseValues(newLayer);

    return newLayer;
}

void LayerVideo::applyOrder(int order)
{
    if(_graphicsItem)
        _graphicsItem->setZValue(order);
}

void LayerVideo::applyLayerVisible(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerVideo::applyOpacity(qreal opacity)
{
    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

QString LayerVideo::getVideoFile() const
{
    return _filename;
}

QImage LayerVideo::getScreenshot() const
{
    return _layerScreenshot;
}

void LayerVideo::dmInitialize(QGraphicsScene* scene)
{
    if(!scene)
        return;

    _dmScene = scene;

    if(!_layerScreenshot.isNull())
    {
        handleScreenshotReady(_layerScreenshot);
    }
    else
    {
        VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(getVideoFile());
        connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &LayerVideo::handleScreenshotReady);
        screenshot->retrieveScreenshot();
    }

    Layer::dmInitialize(scene);
}

void LayerVideo::dmUninitialize()
{
    cleanupDM();
}

void LayerVideo::dmUpdate()
{

}

void LayerVideo::playerGLInitialize(PublishGLRenderer* renderer, PublishGLScene* scene)
{
    Q_UNUSED(scene);

    if((!renderer) || (!renderer->getTargetWidget()))
        return;

    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(_filename,
                                           renderer->getTargetWidget()->context(),
                                           renderer->getTargetWidget()->format(),
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget);
    connect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, renderer, &PublishGLRenderer::updateProjectionMatrix);
    connect(this, &LayerVideo::updateProjectionMatrix, renderer, &PublishGLRenderer::updateProjectionMatrix);
    _videoPlayer->restartPlayer();
}

void LayerVideo::playerGLUninitialize()
{
    cleanupPlayer();
}

bool LayerVideo::playerGLUpdate()
{
    if((!_videoPlayer) || (_videoPlayer->vbObjectsExist()))
        return false;

    _videoPlayer->createVBObjects();
    return _videoPlayer->vbObjectsExist();
}

void LayerVideo::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if((!functions) || (!_videoPlayer))
        return;

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoPlayer->paintGL();
}

void LayerVideo::playerGLResize(int w, int h)
{
    if(!_videoPlayer)
        return;

    _videoPlayer->initializationComplete();
    emit updateProjectionMatrix();
}

void LayerVideo::initialize(const QSize& layerSize)
{

}

void LayerVideo::uninitialize()
{

}

void LayerVideo::handleScreenshotReady(const QImage& image)
{
    if((!_dmScene) || (_graphicsItem) || (image.isNull()))
        return;

    _layerScreenshot = image;

    _graphicsItem = _dmScene->addPixmap(QPixmap::fromImage(getScreenshot()));
    if(_graphicsItem)
    {
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
    }
}

void LayerVideo::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    // todo
}

void LayerVideo::cleanupDM()
{
    _dmScene = nullptr;

    if(_graphicsItem)
    {
        if(_graphicsItem->scene())
            _graphicsItem->scene()->removeItem(_graphicsItem);

        delete _graphicsItem;
        _graphicsItem = nullptr;
    }
}

void LayerVideo::cleanupPlayer()
{
    if(_videoPlayer)
    {
        disconnect(_videoPlayer, nullptr, nullptr, nullptr);
        VideoPlayerGLPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }

    disconnect(this, &LayerVideo::updateProjectionMatrix, nullptr, nullptr);
}
