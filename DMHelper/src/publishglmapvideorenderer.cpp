#include "publishglmapvideorenderer.h"
#include "map.h"
#include "videoplayerglplayer.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QDebug>


#ifdef USE_SCREENSHOT_ONLY
#include "videoplayerglscreenshot.h"
#include "battleglbackground.h"
#endif

PublishGLMapVideoRenderer::PublishGLMapVideoRenderer(Map* map, QObject *parent) :
    PublishGLMapRenderer(map, parent),
    _videoPlayer(nullptr)

    #ifdef USE_SCREENSHOT_ONLY
    , _backgroundObject(nullptr),
      _backgroundImage()
    #endif
{
}

PublishGLMapVideoRenderer::~PublishGLMapVideoRenderer()
{
}

void PublishGLMapVideoRenderer::cleanup()
{
#ifdef USE_SCREENSHOT_ONLY
    delete _backgroundObject;
    _backgroundObject = nullptr;
#endif

    delete _videoPlayer;
    _videoPlayer = nullptr;

    PublishGLMapRenderer::cleanup();
}

#ifdef USE_SCREENSHOT_ONLY
void PublishGLMapVideoRenderer::handleScreenshotReady(const QImage& image)
{
    qDebug() << "[PublishGLMapVideoRenderer] Screenshot received: " << image.size();

    if(image.isNull())
        return;

    _backgroundImage = image;
    emit updateWidget();
}
#endif

void PublishGLMapVideoRenderer::initializeBackground()
{
    if(!_map)
        return;

#ifdef USE_SCREENSHOT_ONLY
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
#endif
}

void PublishGLMapVideoRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

#ifdef USE_SCREENSHOT_ONLY
    if(_backgroundObject)
        updateProjectionMatrix();
#else
    if(!_videoPlayer)
        return;

    _videoPlayer->targetResized(_targetSize);
    _videoPlayer->initializationComplete();
#endif
}

void PublishGLMapVideoRenderer::paintBackground(QOpenGLFunctions* functions)
{
#ifdef USE_SCREENSHOT_ONLY
    if((!_backgroundObject) && (!_backgroundImage.isNull()))
        _backgroundObject = new BattleGLBackground(nullptr, _backgroundImage, GL_NEAREST);

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
#endif
}

QSizeF PublishGLMapVideoRenderer::getBackgroundSize()
{
#ifdef USE_SCREENSHOT_ONLY
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
#else
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
#endif
}

QImage PublishGLMapVideoRenderer::getLastScreenshot()
{
#ifdef USE_SCREENSHOT_ONLY
    return _backgroundImage;
#else
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
#endif
}