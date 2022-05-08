#include "publishgltextvideorenderer.h"
#include "videoplayerglplayer.h"
#include "encountertext.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QDebug>

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
#include "videoplayerglscreenshot.h"
#include "publishglbattlebackground.h"
#endif

PublishGLTextVideoRenderer::PublishGLTextVideoRenderer(EncounterText* encounter, QImage textImage, QObject *parent) :
    PublishGLTextRenderer(encounter, textImage, parent),
    _videoPlayer(nullptr)

    #ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    , _backgroundObject(nullptr),
      _backgroundImage()
    #endif
{
}

PublishGLTextVideoRenderer::~PublishGLTextVideoRenderer()
{
    PublishGLTextVideoRenderer::cleanup();
}

void PublishGLTextVideoRenderer::cleanup()
{
    cleanupBackground();
    PublishGLTextRenderer::cleanup();
}

QSizeF PublishGLTextVideoRenderer::getBackgroundSize()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
#else
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
#endif
}

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
void PublishGLTextVideoRenderer::handleScreenshotReady(const QImage& image)
{
    qDebug() << "[PublishGLTextVideoRenderer] Screenshot received: " << image.size();

    if(image.isNull())
        return;

    _backgroundImage = image;
    rewind();
    emit updateWidget();
}
#endif

void PublishGLTextVideoRenderer::play()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
#else
    initializeBackground();
#endif

    PublishGLTextRenderer::play();
}

void PublishGLTextVideoRenderer::stop()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
#else
    cleanupBackground();
#endif

    PublishGLTextRenderer::stop();
}

void PublishGLTextVideoRenderer::initializeBackground()
{
    if(!_encounter)
        return;

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_encounter->getImageFile());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &PublishGLTextVideoRenderer::handleScreenshotReady);
    screenshot->retrieveScreenshot();
#else
    if(!_videoPlayer)
    {
        // Create the objects
        _videoPlayer = new VideoPlayerGLPlayer(_encounter->getImageFile(),
                                               _targetWidget->context(),
                                               _targetWidget->format(),
                                               true,
                                               false);
        connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLTextVideoRenderer::updateWidget);
        connect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, this, &PublishGLTextVideoRenderer::updateProjectionMatrix);
        connect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, this, &PublishGLTextVideoRenderer::startScrollingTimer);
        _videoPlayer->restartPlayer();
    }
#endif
}

bool PublishGLTextVideoRenderer::isBackgroundReady()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject != nullptr;
#else
    return ((_videoPlayer) && (_videoPlayer->vbObjectsExist()));
#endif
}

void PublishGLTextVideoRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    if(_backgroundObject)
        updateProjectionMatrix();
#else
    if(!_videoPlayer)
        return;

    _videoPlayer->initializationComplete();
    updateProjectionMatrix();
#endif
}

void PublishGLTextVideoRenderer::paintBackground(QOpenGLFunctions* functions)
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
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

void PublishGLTextVideoRenderer::updateBackground()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    if((!_backgroundObject) && (!_backgroundImage.isNull()))
    {
        _backgroundObject = new PublishGLBattleBackground(nullptr, _backgroundImage, GL_NEAREST);
        updateProjectionMatrix();
    }
#else
    if(!_videoPlayer)
        return;

    if(_videoPlayer->vbObjectsExist())
        _videoPlayer->cleanupVBObjects();

    _videoPlayer->createVBObjects();
    _scene.deriveSceneRectFromSize(getBackgroundSize());

#endif
}

void PublishGLTextVideoRenderer::cleanupBackground()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundImage;
#else
    if(_videoPlayer)
    {
        disconnect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLTextVideoRenderer::updateWidget);
        disconnect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, this, &PublishGLTextVideoRenderer::updateProjectionMatrix);
        disconnect(_videoPlayer, &VideoPlayerGLPlayer::vbObjectsCreated, this, &PublishGLTextVideoRenderer::startScrollingTimer);
        VideoPlayerGLPlayer* deletePlayer = _videoPlayer;
        _videoPlayer = nullptr;
        deletePlayer->stopThenDelete();
    }
#endif
}

QImage PublishGLTextVideoRenderer::getLastScreenshot()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    delete _backgroundObject;
    _backgroundObject = nullptr;
#else
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
#endif
}
