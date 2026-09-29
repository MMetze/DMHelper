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

#ifdef LAYERVIDEO_PERF_STATS
static const int LAYERVIDEO_STATS_REPORT_INTERVAL_MS = 5000;
static const qint64 LAYERVIDEO_NSECS_PER_MSEC = 1000000;
#endif

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
    _looping(true),
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

    if(element.hasAttribute("looping"))
        _looping = static_cast<bool>(element.attribute("looping").toInt());

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

bool LayerVideo::isLooping() const
{
    return _looping;
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

#ifdef LAYERVIDEO_PERF_STATS
    reportStatsIfDue();
#endif

    if(!_videoObject)
    {
        if((!_videoPlayer->isNewImage()) && (getScreenshot().isNull()))
            return;

        if(_videoPlayer->lockMutex())
        {
            QImage* playerImage = _videoPlayer->getLockedImage();
            if(playerImage)
            {
#ifdef LAYERVIDEO_PERF_STATS
                QElapsedTimer uploadTimer;
                uploadTimer.start();
#endif
                // The display buffer is consumer-exclusive under the triple-buffer scheme (write/ready/display),
                // so the GL upload reads it directly without a defensive copy. Frames are already Format_RGBA8888
                // and top-down, so the FOW fast-path flags skip the convert and CPU-flip copies.
                _videoObject = new PublishGLBattleBackground(nullptr, *playerImage, GL_NEAREST, true, false);
                QPoint pointTopLeft = _scene ? _scene->getSceneRect().toRect().topLeft() : QPoint();
                _videoObject->setPosition(QPoint(pointTopLeft.x() + _position.x(), -pointTopLeft.y() - _position.y()));
                _videoObject->setTargetSize(_size);
#ifdef LAYERVIDEO_PERF_STATS
                recordUploadStat(uploadTimer.nsecsElapsed());
#endif
            }
            _videoPlayer->clearNewImage();
            _videoPlayer->unlockMutex();
        }
    }
    else if(_videoPlayer->isNewImage())
    {
        if(_videoPlayer->lockMutex())
        {
            QImage* playerImage = _videoPlayer->getLockedImage();
            if(playerImage)
            {
#ifdef LAYERVIDEO_PERF_STATS
                QElapsedTimer uploadTimer;
                uploadTimer.start();
#endif
                // Size changes (player restart / new media) must reallocate the texture; the steady state
                // sub-uploads in place to avoid a per-frame glTexImage2D reallocation
                if(_videoObject->getSize() != playerImage->size())
                    _videoObject->updateImage(*playerImage);
                else
                    _videoObject->updateImageRegion(*playerImage, QRect(QPoint(0, 0), playerImage->size()));
#ifdef LAYERVIDEO_PERF_STATS
                recordUploadStat(uploadTimer.nsecsElapsed());
#endif
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

    if((!functions) || (!_videoObject))
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

void LayerVideo::setLooping(bool looping)
{
    if(_looping == looping)
        return;

    _looping = looping;
    if(_videoPlayer)
        _videoPlayer->setLooping(looping);
    emit dirty();
}

void LayerVideo::setVideoFile(const QString& filename)
{
    if((filename.isEmpty()) || (_filename == filename))
        return;

    cleanupDM();
    cleanupPlayer();
    clearScreenshot();
    _layerScreenshot = QImage();

    _filename = filename;

    requestScreenshot();
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

    if(!_looping)
        element.setAttribute("looping", 0);

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
            _graphicsItem->setShapeMode(QGraphicsPixmapItem::BoundingRectShape);
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
    connect(_videoGLPlayer, &VideoPlayerGLPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget, Qt::QueuedConnection);
    connect(_videoGLPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, renderer, &PublishGLRenderer::updateProjectionMatrix);
    _videoGLPlayer->restartPlayer();
#else
    _videoPlayer = new VideoPlayer(_filename, QSize(), true, _playAudio);
    _videoPlayer->setLooping(_looping);
    connect(_videoPlayer, &VideoPlayer::frameAvailable, renderer, &PublishGLRenderer::updateWidget, Qt::QueuedConnection);
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

#ifdef LAYERVIDEO_PERF_STATS
void LayerVideo::recordUploadStat(qint64 uploadNs)
{
    ++_statUploadCount;
    _statUploadAccumNs += uploadNs;
    if(uploadNs > _statUploadMaxNs)
        _statUploadMaxNs = uploadNs;

    if(_statIntervalTimer.isValid())
    {
        qint64 intervalNs = _statIntervalTimer.nsecsElapsed();
        ++_statIntervalCount;
        _statIntervalAccumNs += intervalNs;
        if((_statIntervalMinNs == 0) || (intervalNs < _statIntervalMinNs))
            _statIntervalMinNs = intervalNs;
        if(intervalNs > _statIntervalMaxNs)
            _statIntervalMaxNs = intervalNs;
    }
    _statIntervalTimer.start();
}

void LayerVideo::reportStatsIfDue()
{
    if(!_statReportTimer.isValid())
    {
        _statReportTimer.start();
        return;
    }

    if(_statReportTimer.elapsed() < LAYERVIDEO_STATS_REPORT_INTERVAL_MS)
        return;

    const unsigned int decoded = _videoPlayer ? _videoPlayer->getStatFramesDecoded() : 0;
    const unsigned int dropped = _videoPlayer ? _videoPlayer->getStatFramesDropped() : 0;
    const qint64 decodeAccumNs = _videoPlayer ? _videoPlayer->getStatDecodeIntervalAccumNs() : 0;
    const unsigned int decodeCount = _videoPlayer ? _videoPlayer->getStatDecodeIntervalCount() : 0;
    const qint64 decodeMaxNs = _videoPlayer ? _videoPlayer->takeStatDecodeIntervalMaxNs() : 0;
    // A restarted player resets its counters, so guard against negative deltas
    const unsigned int decodedDelta = (decoded >= _statLastDecoded) ? decoded - _statLastDecoded : decoded;
    const unsigned int droppedDelta = (dropped >= _statLastDropped) ? dropped - _statLastDropped : dropped;
    const qint64 decodeAccumDeltaNs = (decodeAccumNs >= _statLastDecodeAccumNs) ? decodeAccumNs - _statLastDecodeAccumNs : decodeAccumNs;
    const unsigned int decodeCountDelta = (decodeCount >= _statLastDecodeCount) ? decodeCount - _statLastDecodeCount : decodeCount;
    _statLastDecoded = decoded;
    _statLastDropped = dropped;
    _statLastDecodeAccumNs = decodeAccumNs;
    _statLastDecodeCount = decodeCount;

    const double avgUploadMs = (_statUploadCount > 0) ? static_cast<double>(_statUploadAccumNs) / static_cast<double>(_statUploadCount) / LAYERVIDEO_NSECS_PER_MSEC : 0.0;
    const double maxUploadMs = static_cast<double>(_statUploadMaxNs) / LAYERVIDEO_NSECS_PER_MSEC;
    const double minIntervalMs = static_cast<double>(_statIntervalMinNs) / LAYERVIDEO_NSECS_PER_MSEC;
    const double avgIntervalMs = (_statIntervalCount > 0) ? static_cast<double>(_statIntervalAccumNs) / static_cast<double>(_statIntervalCount) / LAYERVIDEO_NSECS_PER_MSEC : 0.0;
    const double maxIntervalMs = static_cast<double>(_statIntervalMaxNs) / LAYERVIDEO_NSECS_PER_MSEC;
    const double avgDecodeIntervalMs = (decodeCountDelta > 0) ? static_cast<double>(decodeAccumDeltaNs) / static_cast<double>(decodeCountDelta) / LAYERVIDEO_NSECS_PER_MSEC : 0.0;
    const double maxDecodeIntervalMs = static_cast<double>(decodeMaxNs) / LAYERVIDEO_NSECS_PER_MSEC;

    qDebug() << "[LayerVideo] perf:" << getName()
             << "decoded:" << decodedDelta
             << "uploaded:" << _statUploadCount
             << "dropped:" << droppedDelta
             << "upload ms avg/max:" << avgUploadMs << "/" << maxUploadMs
             << "interval ms min/avg/max:" << minIntervalMs << "/" << avgIntervalMs << "/" << maxIntervalMs
             << "decode interval ms avg/max:" << avgDecodeIntervalMs << "/" << maxDecodeIntervalMs;

    _statUploadCount = 0;
    _statUploadAccumNs = 0;
    _statUploadMaxNs = 0;
    _statIntervalCount = 0;
    _statIntervalAccumNs = 0;
    _statIntervalMinNs = 0;
    _statIntervalMaxNs = 0;
    _statReportTimer.start();
}
#endif
