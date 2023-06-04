#include "layervideo.h"
#include "layerscene.h"

#ifdef LAYERVIDEO_USE_OPENGL
    #include "videoplayerglplayer.h"
    #include "videoplayerglscreenshot.h"
#else
    #include "videoplayer.h"
    #include "publishglbattlebackground.h"
    #include "videoplayerscreenshot.h"
#endif

#include "publishglrenderer.h"
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QOpenGLWidget>

LayerVideo::LayerVideo(const QString& name, const QString& filename, int order, QObject *parent) :
    Layer{name, order, parent},
    _graphicsItem(nullptr),
#ifdef LAYERVIDEO_USE_OPENGL
    _videoGLPlayer(nullptr),
    _screenshot(nullptr),
#else
    _videoPlayer(nullptr),
    _screenshot(nullptr),
    _videoObject(),
    _playerSize(),
#endif
    _scene(nullptr),
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
    _filename = element.attribute("videoFile");

    Layer::inputXML(element, isImport);
}

QRectF LayerVideo::boundingRect() const
{
    return QRectF(_position, _size);
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
    _opacityReference = opacity;

    if(_graphicsItem)
        _graphicsItem->setOpacity(opacity);
}

void LayerVideo::applyPosition(const QPoint& position)
{
    if(_graphicsItem)
        _graphicsItem->setPos(position);

#ifdef LAYERVIDEO_USE_OPENGL
    TODO, how should this work with: VideoPlayerGLPlayer* _videoGLPlayer;
#else
    if(_videoObject)
    {
        QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        _videoObject->setPosition(QPoint(pointTopLeft.x() + position.x(), -pointTopLeft.y() - position.y()));
    }
#endif

}

void LayerVideo::applySize(const QSize& size)
{
    if(_graphicsItem)
    {
        qreal xScale = static_cast<qreal>(size.width()) / _layerScreenshot.width();
        qreal yScale = static_cast<qreal>(size.height()) / _layerScreenshot.height();
        _graphicsItem->setScale(qMin(xScale, yScale));
    }
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

    requestScreenshot();

    Layer::dmInitialize(scene);
}

void LayerVideo::dmUninitialize()
{
    cleanupDM();
}

void LayerVideo::dmUpdate()
{

}

void LayerVideo::playerGLInitialize(PublishGLScene* scene)
{
    Q_UNUSED(scene);

    if(!_layerScene)
        return;

    PublishGLRenderer* renderer = _layerScene->getRenderer();
    if((!renderer) || (!renderer->getTargetWidget()))
        return;

#ifdef LAYERVIDEO_USE_OPENGL
    if(_videoGLPlayer)
        return;
#else
    if(_videoPlayer)
        return;
#endif

    _scene = scene;

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
    //_videoPlayer = new VideoPlayer(_filename, _playerSize, true, false);
    _videoPlayer = new VideoPlayer(_filename, QSize(), true, false);
    connect(_videoPlayer, &VideoPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget);
    _videoPlayer->restartPlayer();
#endif

    Layer::playerGLInitialize(scene);
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

    TODO: update this to position correctly
    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoGLPlayer->paintGL();
#else
    if((!_videoPlayer) || (!_videoPlayer->getImage()))
        return;

    functions->glUseProgram(_shaderProgramRGBA);

    if(!_videoObject)
    {
        if(!_videoPlayer->isNewImage())
            return;

        _videoObject = new PublishGLBattleBackground(nullptr, *(_videoPlayer->getImage()), GL_NEAREST);
        QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
        _videoObject->setPosition(QPoint(pointTopLeft.x() + _position.x(), -pointTopLeft.y() - _position.y()));
    }
    else if(_videoPlayer->isNewImage())
    {
        _videoObject->setImage(*(_videoPlayer->getImage()));
    }

    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _videoObject->getMatrixData());
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);

    //QMatrix4x4 modelMatrix;
    //functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoObject->paintGL();

    functions->glUseProgram(_shaderProgramRGB);
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

bool LayerVideo::playerIsInitialized()
{
#ifdef LAYERVIDEO_USE_OPENGL
    return _videoGLPlayer != nullptr;
#else
    return _videoPlayer != nullptr;
#endif
}

void LayerVideo::initialize(const QSize& sceneSize)
{
    if(_size.isEmpty())
    {
        _size = sceneSize;
        requestScreenshot();
    }
}

void LayerVideo::uninitialize()
{
    _layerScreenshot = QImage();
    clearScreenshot();
}

void LayerVideo::handleScreenshotReady(const QImage& image)
{
    if(image.isNull())
        return;

    qDebug() << "[LayerVideo] Screenshot received for video: " << getVideoFile() << ", " << image;
    _layerScreenshot = image.copy();

    if(_dmScene)
        createGraphicsItem(_size.isEmpty() ? _layerScreenshot.size() : _size);

    if(_size.isEmpty())
        setSize(_layerScreenshot.size());
}

void LayerVideo::requestScreenshot()
{
    if(!_layerScreenshot.isNull())
    {
        handleScreenshotReady(_layerScreenshot);
        return;
    }

    if(_screenshot)
        return;

    qDebug() << "[LayerVideo] New screenshot needed for video: " << getVideoFile();
#ifdef LAYERVIDEO_USE_OPENGL
    _screenshot = new VideoPlayerGLScreenshot(getVideoFile());
    connect(_screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &LayerVideo::handleScreenshotReady);
    connect(_screenshot, &QObject::destroyed, this, &LayerVideo::clearScreenshot);
    _screenshot->retrieveScreenshot();
#else
    _screenshot = new VideoPlayerScreenshot(getVideoFile());
    connect(_screenshot, &VideoPlayerScreenshot::screenshotReady, this, &LayerVideo::handleScreenshotReady);
    connect(_screenshot, &QObject::destroyed, this, &LayerVideo::clearScreenshot);
    _screenshot->retrieveScreenshot();
#endif
}

void LayerVideo::clearScreenshot()
{
    if(!_screenshot)
        return;

    qDebug() << "[LayerVideo] Clearing screenshot requester";

#ifdef LAYERVIDEO_USE_OPENGL
    disconnect(_screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &LayerVideo::handleScreenshotReady);
#else
    disconnect(_screenshot, &VideoPlayerScreenshot::screenshotReady, this, &LayerVideo::handleScreenshotReady);
#endif
    disconnect(_screenshot, &QObject::destroyed, this, &LayerVideo::clearScreenshot);

    _screenshot = nullptr;
}

void LayerVideo::internalOutputXML(QDomDocument &doc, QDomElement &element, QDir& targetDirectory, bool isExport)
{
    element.setAttribute("videoFile", targetDirectory.relativeFilePath(_filename));

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerVideo::createGraphicsItem(const QSize& size)
{
    if((!_dmScene) || (_graphicsItem) || (size.isEmpty()))
        return;

    _graphicsItem = _dmScene->addPixmap(QPixmap::fromImage(_layerScreenshot));
    if(_graphicsItem)
    {
        _graphicsItem->setPos(_position);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
        _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
        _graphicsItem->setZValue(getOrder());

        applySize(size);
    }
}

void LayerVideo::cleanupDM()
{
    if(_dmScene)
    {
        if(_graphicsItem)
        {
            _dmScene->removeItem(_graphicsItem);
            delete _graphicsItem;
            _graphicsItem = nullptr;
        }
        _dmScene = nullptr;
    }

    clearScreenshot();
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

    _scene = nullptr;
}
