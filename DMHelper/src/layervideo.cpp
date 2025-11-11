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
#include <QDebug>

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
    _playAudio(true),
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
    if(element.hasAttribute("videoFile"))
        _filename = element.attribute("videoFile");

    if(element.hasAttribute("playAudio"))
        _playAudio = static_cast<bool>(element.attribute("playAudio").toInt());

    Layer::inputXML(element, isImport);
}

QRectF LayerVideo::boundingRect() const
{
    return QRectF(_position, _size);
}

QImage LayerVideo::getLayerIcon() const
{
    QImage screenshot = getScreenshot();
    return screenshot.isNull() ? QImage(":/img/data/icon_play.png") : screenshot;
}

bool LayerVideo::hasAudio() const
{
    return true;
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

void LayerVideo::applyLayerVisibleDM(bool layerVisible)
{
    if(_graphicsItem)
        _graphicsItem->setVisible(layerVisible);
}

void LayerVideo::applyLayerVisiblePlayer(bool layerVisible)
{
    if(!layerVisible)
        cleanupPlayerObject();
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
        updateImage(size);

#ifdef LAYERVIDEO_USE_OPENGL
    TODO, how should this work with: VideoPlayerGLPlayer* _videoGLPlayer;
#else
    if(_videoObject)
        _videoObject->setTargetSize(size);
#endif
}

QString LayerVideo::getVideoFile() const
{
    return _filename;
}

bool LayerVideo::getPlayAudio() const
{
    return _playAudio;
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

    if(_layerScreenshot.isNull())
        requestScreenshot();
    else
        updateImage();

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

    if(!renderer)
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

    if(_layerVisiblePlayer)
        createPlayerObjectGL(renderer);

    Layer::playerGLInitialize(renderer, scene);
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
    if(!functions)
        return;

#ifdef LAYERVIDEO_USE_OPENGL
    TODO: handle layer visibility like below
    if(!_videoGLPlayer)
        return;

    TODO: update this to position correctly
    QMatrix4x4 modelMatrix;
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData(), modelMatrix);
    functions->glUniformMatrix4fv(defaultModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoGLPlayer->paintGL();
#else

    if(!_videoPlayer)
        return;

    DMH_DEBUG_OPENGL_PAINTGL();

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGBA);
    functions->glUseProgram(_shaderProgramRGBA);

    if(!_videoPlayer->isNewImage())
    {
        if(_videoPlayer->lockMutex())
        {
            qDebug() << "[LayerVideo] Getting new image from video player: " << _videoPlayer << ", " << this;
            QImage* playerImage = _videoPlayer->getLockedImage();
            if(playerImage)
            {
                QImage imageCopy = playerImage->copy();
                if(!_videoObject)
                {
                    _videoObject = new PublishGLBattleBackground(nullptr, imageCopy, GL_NEAREST);
                    QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
                    _videoObject->setPosition(QPoint(pointTopLeft.x() + _position.x(), -pointTopLeft.y() - _position.y()));
                    _videoObject->setTargetSize(_size);
                }
                else
                {
                    _videoObject->updateImage(imageCopy);
                }
            }
            _videoPlayer->clearNewImage();
            _videoPlayer->unlockMutex();
        }
    }

    if(!_videoObject)
        return;

    playerGLSetUniforms(functions, defaultModelMatrix, projectionMatrix);

    _videoObject->paintGL(functions, projectionMatrix);

    DMH_DEBUG_OPENGL_glUseProgram(_shaderProgramRGB);
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
    Q_UNUSED(sceneSize);

    requestScreenshot();
}

void LayerVideo::uninitialize()
{
    _layerScreenshot = QImage();
    clearScreenshot();
}

void LayerVideo::playerGLSetUniforms(QOpenGLFunctions* functions, GLint defaultModelMatrix, const GLfloat* projectionMatrix)
{
    Q_UNUSED(defaultModelMatrix);

    if(!functions)
        return;

    DMH_DEBUG_OPENGL_glUniformMatrix4fv4(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glUniformMatrix4fv(_shaderProjectionMatrixRGBA, 1, GL_FALSE, projectionMatrix);
    functions->glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    DMH_DEBUG_OPENGL_glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _videoObject->getMatrixData(), _videoObject->getMatrix());
    functions->glUniformMatrix4fv(_shaderModelMatrixRGBA, 1, GL_FALSE, _videoObject->getMatrixData());
    DMH_DEBUG_OPENGL_glUniform1f(_shaderAlphaRGBA, _opacityReference);
    functions->glUniform1f(_shaderAlphaRGBA, _opacityReference);
}

void LayerVideo::setPlayAudio(bool playAudio)
{
    if(_playAudio == playAudio)
        return;

    _playAudio = playAudio;
    if(_videoPlayer)
        _videoPlayer->setPlayingAudio(playAudio);
    emit dirty();
}

void LayerVideo::handleScreenshotReady(const QImage& image)
{
    if((image.isNull()) || (_layerScreenshot == image))
        return;

    qDebug() << "[LayerVideo] Screenshot received for video: " << getVideoFile() << ", " << image;
    _layerScreenshot = image.copy();

    if(_size.isEmpty())
        setSize(getScreenshot().size());

    updateImage();

    emit screenshotAvailable();
    emit dirty();
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

    if(!_playAudio)
        element.setAttribute("playAudio", 0);

    Layer::internalOutputXML(doc, element, targetDirectory, isExport);
}

void LayerVideo::updateImage(const QSize& size)
{
    if(!_dmScene)
        return;

    QImage screenshot = getScreenshot();
    if(screenshot.isNull())
        return;

    QSize scaleSize = size.isEmpty() ? getSize() : size;
    if(scaleSize.isEmpty())
        return;

    QImage scaledImage = getScreenshot().scaled(scaleSize);
    if(!_graphicsItem)
    {
        _graphicsItem = _dmScene->addPixmap(QPixmap::fromImage(scaledImage));
        if(_graphicsItem)
        {
            _graphicsItem->setPos(_position);
            _graphicsItem->setFlag(QGraphicsItem::ItemIsMovable, false);
            _graphicsItem->setFlag(QGraphicsItem::ItemIsSelectable, false);
            _graphicsItem->setZValue(getOrder());
        }
    }
    else
    {
        _graphicsItem->setPixmap(QPixmap::fromImage(scaledImage));
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

    LayerVideo::clearScreenshot();
}

void LayerVideo::createPlayerObjectGL(PublishGLRenderer* renderer)
{
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
    _videoPlayer = new VideoPlayer(_filename, QSize(), true, _playAudio);
    connect(_videoPlayer, &VideoPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget);
    _videoPlayer->restartPlayer();
#endif
}

void LayerVideo::cleanupPlayerObject()
{
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

void LayerVideo::cleanupPlayer()
{    
    disconnect(this, &LayerVideo::updateProjectionMatrix, nullptr, nullptr);

    cleanupPlayerObject();

    _scene = nullptr;
}
