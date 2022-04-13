#include "publishglmapvideorenderer.h"
#include "map.h"
#include "videoplayerglplayer.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QDebug>

#include "publishglbattlebackground.h"

#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
#include "videoplayerglscreenshot.h"
#include "publishglbattlebackground.h"
#endif

PublishGLMapVideoRenderer::PublishGLMapVideoRenderer(Map* map, QObject *parent) :
    PublishGLMapRenderer(map, parent),
    _videoPlayer(nullptr)

  , _tempObject(nullptr),
    _tempImage()


    #ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    , _backgroundObject(nullptr),
      _backgroundImage()
    #endif
{
}

void PublishGLMapVideoRenderer::cleanup()
{
#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    delete _backgroundObject;
    _backgroundObject = nullptr;
#endif

    delete _tempObject;
    _tempObject = nullptr;

    delete _videoPlayer;
    _videoPlayer = nullptr;

    PublishGLMapRenderer::cleanup();
}

#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
void PublishGLMapVideoRenderer::handleScreenshotReady(const QImage& image)
{
    qDebug() << "[PublishGLMapVideoRenderer] Screenshot received: " << image.size();

    if(image.isNull())
        return;

    _backgroundImage = image;
    _updateFow = true;
    emit updateWidget();
}
#endif

void PublishGLMapVideoRenderer::initializeBackground()
{
    if(!_map)
        return;

#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_map->getFileName());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &PublishGLMapVideoRenderer::handleScreenshotReady);
    screenshot->retrieveScreenshot();
#else
    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(_map->getFileName(),
                                           _targetWidget->context(),
                                           _targetWidget->format(),
                                           _targetSize,
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLMapVideoRenderer::updateWidget);
    connect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, this, &PublishGLMapVideoRenderer::updateProjectionMatrix);
    _videoPlayer->restartPlayer();

    _tempImage.load("mountainruins.png");
    _tempObject = new PublishGLBattleBackground(nullptr, _tempImage, GL_NEAREST);

#endif
}

bool PublishGLMapVideoRenderer::isBackgroundReady()
{
#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject != nullptr;
#else
    return _videoPlayer != nullptr;
#endif
}

void PublishGLMapVideoRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    if(_backgroundObject)
        updateProjectionMatrix();
#else
    if(!_videoPlayer)
        return;

    _videoPlayer->targetResized(_targetSize);
    _videoPlayer->initializationComplete();
    updateProjectionMatrix();
#endif
}

void PublishGLMapVideoRenderer::paintBackground(QOpenGLFunctions* functions)
{
#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    if(_backgroundObject)
    {
        functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, _backgroundObject->getMatrixData());
        _backgroundObject->paintGL();
    }
#else
    if(!_videoPlayer)
        return;

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoPlayer->paintGL();
    //_tempObject->paintGL();
#endif
}

QSizeF PublishGLMapVideoRenderer::getBackgroundSize()
{
#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
#else
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
#endif
}

void PublishGLMapVideoRenderer::updateBackground()
{
#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    if((!_backgroundObject) && (!_backgroundImage.isNull()))
    {
        _backgroundObject = new PublishGLBattleBackground(nullptr, _backgroundImage, GL_NEAREST);
        updateFoW();
        updateProjectionMatrix();
    }
#endif
}

QImage PublishGLMapVideoRenderer::getLastScreenshot()
{
#ifdef MAPVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundImage;
#else
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
#endif
}
