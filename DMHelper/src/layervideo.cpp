#include "layervideo.h"
#include "layerscene.h"

#ifdef LAYERVIDEO_USE_OPENGL
    #include "videoplayerglplayer.h"
#else
    #include "videoplayer.h"
    #include "publishglbattlebackground.h"
#endif

#include "videoplayerglscreenshot.h"
#include "publishglrenderer.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLWidget>

LayerVideo::LayerVideo(const QString& name, const QString& filename, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
#ifdef LAYERVIDEO_USE_OPENGL
    _videoGLPlayer(nullptr),
#else
    _videoPlayer(nullptr),
    _videoObject(),
    _playerSize(),
#endif
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

#ifdef LAYERVIDEO_USE_OPENGL
    if(_videoGLPlayer)
        return;
#else
    if(_videoPlayer)
        return;
#endif

    connect(this, &LayerVideo::updateProjectionMatrix, renderer, &PublishGLRenderer::updateProjectionMatrix);

    // Create the objects
#ifdef LAYERVIDEO_USE_OPENGL
    _videoGLPlayer = new VideoPlayerGLPlayer(_filename,
                                             renderer->getTargetWidget()->context(),
                                             renderer->getTargetWidget()->format(),
                                             true,
                                             false);
    connect(_videoGLPlayer, &VideoPlayerGLPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget);
    connect(_videoGLPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, renderer, &PublishGLRenderer::updateProjectionMatrix);
    _videoGLPlayer->restartPlayer();
#else
    _videoPlayer = new VideoPlayer(_filename, _playerSize, true, false);
    connect(_videoPlayer, &VideoPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget);
    _videoPlayer->restartPlayer();
#endif
}

void LayerVideo::playerGLUninitialize()
{
    cleanupPlayer();
}

bool LayerVideo::playerGLUpdate()
{
#ifdef LAYERVIDEO_USE_OPENGL
    if((!_videoGLPlayer) || (_videoGLPlayer->vbObjectsExist()))
        return false;

    _videoGLPlayer->createVBObjects();
    return _videoGLPlayer->vbObjectsExist();
#else
    return false;
#endif
}

void LayerVideo::playerGLPaint(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(projectionMatrix);

    if(!functions)
        return;

#ifdef LAYERVIDEO_USE_OPENGL
    if(!_videoGLPlayer)
        return;

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoGLPlayer->paintGL();
#else
    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    if(!_videoObject)
    {
        if(!_videoPlayer->isNewImage())
            return;

        _videoObject = new PublishGLBattleBackground(nullptr, *(_videoPlayer->getImage()), GL_NEAREST);
    }
    else if(_videoPlayer->isNewImage())
    {
        _videoObject->setImage(*(_videoPlayer->getImage()));
    }

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoObject->paintGL();
#endif
}

void LayerVideo::playerGLResize(int w, int h)
{
#ifdef LAYERVIDEO_USE_OPENGL
    if(!_videoGLPlayer)
        return;

    _videoGLPlayer->initializationComplete();
#else
    _playerSize = QSize(w, h);
#endif
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
    if((!_dmScene) || (_graphicsItem) || (image.isNull()) ||(!getLayerScene()))
        return;

    _layerScreenshot = image;

    _graphicsItem = _dmScene->addPixmap(QPixmap::fromImage(_layerScreenshot));
    if(_graphicsItem)
    {
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());
        _graphicsItem->setOffset(-static_cast<qreal>(_layerScreenshot.width())/2.0, -static_cast<qreal>(_layerScreenshot.height())/2.0);

        QSizeF sceneSize = getLayerScene()->sceneSize();

        qreal xScale = sceneSize.width() / _layerScreenshot.width();
        qreal yScale = sceneSize.height() / _layerScreenshot.height();
        _graphicsItem->setScale(qMin(xScale, yScale));

        _graphicsItem->setPos(sceneSize.width() / 2.0, sceneSize.height() / 2.0);
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
    disconnect(this, &LayerVideo::updateProjectionMatrix, nullptr, nullptr);

#ifdef LAYERVIDEO_USE_OPENGL
    if(_videoGLPlayer)
    {
        disconnect(_videoGLPlayer, nullptr, nullptr, nullptr);
        VideoPlayerGLPlayer* deletePlayer = _videoGLPlayer;
        _videoGLPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }
#else
    if(_videoPlayer)
    {
        disconnect(_videoPlayer, nullptr, nullptr, nullptr);
        VideoPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }

    delete _videoObject;
    _videoObject = nullptr;
    _playerSize = QSize();
#endif
}
