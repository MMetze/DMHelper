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
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    delete _backgroundObject;
    _backgroundObject = nullptr;
#endif

    delete _videoPlayer;
    _videoPlayer = nullptr;

    PublishGLTextRenderer::cleanup();
}

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
void PublishGLTextVideoRenderer::handleScreenshotReady(const QImage& image)
{
    qDebug() << "[PublishGLTextVideoRenderer] Screenshot received: " << image.size();

    if(image.isNull())
        return;

    _backgroundImage = image;
    emit updateWidget();
}
#endif

void PublishGLTextVideoRenderer::initializeBackground()
{
    if(!_encounter)
        return;

#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_encounter->getImageFile());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &PublishGLTextVideoRenderer::handleScreenshotReady);
    screenshot->retrieveScreenshot();
#else
    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(_encounter->getImageFile(),
                                           _targetWidget->context(),
                                           _targetWidget->format(),
                                           _targetSize,
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLTextVideoRenderer::updateWidget);
#endif
}

bool PublishGLTextVideoRenderer::isBackgroundReady()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject != nullptr;
#else
    return _videoPlayer != nullptr;
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

    _videoPlayer->targetResized(_targetSize);
    _videoPlayer->initializationComplete();
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

QSizeF PublishGLTextVideoRenderer::getBackgroundSize()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
#else
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
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
#endif
}

QImage PublishGLTextVideoRenderer::getLastScreenshot()
{
#ifdef TEXTVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundImage;
#else
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
#endif
}
