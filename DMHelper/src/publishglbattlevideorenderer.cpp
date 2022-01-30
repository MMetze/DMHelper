#include "publishglbattlevideorenderer.h"
#include "publishglbattlebackground.h"
#include "battledialogmodel.h"
#include "videoplayerglplayer.h"
#include "videoplayerglscreenshot.h"
#include "map.h"
#include <QOpenGLWidget>
#include <QOpenGLFunctions>

PublishGLBattleVideoRenderer::PublishGLBattleVideoRenderer(BattleDialogModel* model, QObject *parent) :
    PublishGLBattleRenderer(model, parent),
    _videoPlayer(nullptr)

    #ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    , _backgroundObject(nullptr),
      _backgroundImage()
    #endif
{
}

void PublishGLBattleVideoRenderer::cleanup()
{
#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    delete _backgroundObject;
    _backgroundObject = nullptr;
#endif

    delete _videoPlayer;
    _videoPlayer = nullptr;

    PublishGLBattleRenderer::cleanup();
}

QSizeF PublishGLBattleVideoRenderer::getBackgroundSize()
{
#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject ? _backgroundObject->getSize() : QSizeF();
#else
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
#endif
}

#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
void PublishGLBattleVideoRenderer::handleScreenshotReady(const QImage& image)
{
    qDebug() << "[PublishGLBattleVideoRenderer] Screenshot received: " << image.size();

    if(image.isNull())
        return;

    _backgroundImage = image;
    _updateFow = true;
    emit updateWidget();
    initializationComplete();
}
#endif

void PublishGLBattleVideoRenderer::initializeBackground()
{
    if((!_model) || (!_model->getMap()))
        return;

#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    VideoPlayerGLScreenshot* screenshot = new VideoPlayerGLScreenshot(_model->getMap()->getFileName());
    connect(screenshot, &VideoPlayerGLScreenshot::screenshotReady, this, &PublishGLBattleVideoRenderer::handleScreenshotReady);
    screenshot->retrieveScreenshot();
#else
    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(_model->getMap()->getFileName(),
                                           _targetWidget->context(),
                                           _targetWidget->format(),
                                           _scene.getTargetSize(),
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLBattleVideoRenderer::updateWidget);
    initializationComplete();
#endif
}

bool PublishGLBattleVideoRenderer::isBackgroundReady()
{
#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundObject != nullptr;
#else
    return _videoPlayer != nullptr;
#endif
}

void PublishGLBattleVideoRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    if(_backgroundObject)
        updateProjectionMatrix();
#else
    if(!_videoPlayer)
        return;

    _videoPlayer->targetResized(_scene.getTargetSize());
    _videoPlayer->initializationComplete();
#endif
}

void PublishGLBattleVideoRenderer::paintBackground(QOpenGLFunctions* functions)
{
#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
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

void PublishGLBattleVideoRenderer::updateBackground()
{
#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    if((!_backgroundObject) && (!_backgroundImage.isNull()))
    {
        _backgroundObject = new PublishGLBattleBackground(nullptr, _backgroundImage, GL_NEAREST);
        updateFoW();
        updateProjectionMatrix();
    }
#endif
}

QImage PublishGLBattleVideoRenderer::getLastScreenshot()
{
#ifdef BATTLEVIDEO_USE_SCREENSHOT_ONLY
    return _backgroundImage;
#else
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
#endif
}
