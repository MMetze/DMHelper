#include "publishglmapvideorenderer.h"
#include "map.h"
#include "videoplayerglplayer.h"
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QDebug>

PublishGLMapVideoRenderer::PublishGLMapVideoRenderer(Map* map, QObject *parent) :
    PublishGLMapRenderer(map, parent),
    _videoPlayer(nullptr)
{
}

PublishGLMapVideoRenderer::~PublishGLMapVideoRenderer()
{
}

void PublishGLMapVideoRenderer::cleanup()
{
    delete _videoPlayer;
    _videoPlayer = nullptr;

    PublishGLMapRenderer::cleanup();
}

void PublishGLMapVideoRenderer::initializeBackground()
{
    // Create the objects
    _videoPlayer = new VideoPlayerGLPlayer(_map->getFileName(),
                                           _targetWidget->context(),
                                           _targetWidget->format(),
                                           _targetSize,
                                           true,
                                           false);
    connect(_videoPlayer, &VideoPlayerGLPlayer::frameAvailable, this, &PublishGLMapVideoRenderer::updateWidget);
}

void PublishGLMapVideoRenderer::resizeBackground(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);

    if(!_videoPlayer)
        return;

    _videoPlayer->targetResized(_targetSize);
    _videoPlayer->initializationComplete();
}

void PublishGLMapVideoRenderer::paintBackground(QOpenGLFunctions* functions)
{
    if(!_videoPlayer)
        return;

    QMatrix4x4 modelMatrix;
    functions->glUniformMatrix4fv(_shaderModelMatrix, 1, GL_FALSE, modelMatrix.constData());
    _videoPlayer->paintGL();
}

QSizeF PublishGLMapVideoRenderer::getBackgroundSize()
{
    return _videoPlayer ? _videoPlayer->getSize() : QSizeF();
}

QImage PublishGLMapVideoRenderer::getLastScreenshot()
{
    return _videoPlayer ? _videoPlayer->getLastScreenshot() : QImage();
}
