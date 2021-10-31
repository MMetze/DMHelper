#include "videoplayergl.h"

VideoPlayerGL::VideoPlayerGL(QObject *parent) :
    QObject(parent)
{
}

void VideoPlayerGL::registerNewFrame()
{
}

QSurfaceFormat VideoPlayerGL::getFormat() const
{
    return QSurfaceFormat::defaultFormat();
}

QSize VideoPlayerGL::getSize() const
{
    return QSize();
}

void VideoPlayerGL::videoResized()
{
}

bool VideoPlayerGL::initializeVLC()
{
    return true;
}

bool VideoPlayerGL::startPlayer()
{
    return true;
}

bool VideoPlayerGL::stopPlayer()
{
    return true;
}

