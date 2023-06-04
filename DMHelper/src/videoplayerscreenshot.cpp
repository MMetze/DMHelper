#include "videoplayerscreenshot.h"
#include "dmhcache.h"
#include <QFile>
#include <QDebug>

VideoPlayerScreenshot::VideoPlayerScreenshot(const QString& videoFile, QObject *parent) :
    VideoPlayer{videoFile, QSize(), true, false, parent}
{
    connect(this, &VideoPlayer::screenShotAvailable, this, &VideoPlayerScreenshot::handleScreenshot);
}

VideoPlayerScreenshot::~VideoPlayerScreenshot()
{
    disconnect(this, &VideoPlayer::screenShotAvailable, this, &VideoPlayerScreenshot::handleScreenshot);
}

void VideoPlayerScreenshot::retrieveScreenshot()
{
    // Check if we have a valid video file
    if(_videoFile.isEmpty())
    {
        emit screenshotReady(QImage());
        return;
    }

    // See if we can find a screenshot in the cache
    QString cacheFilePath = DMHCache().getCacheFilePath(_videoFile, QString("png"));
    if((!cacheFilePath.isEmpty()) && (QFile::exists(cacheFilePath)))
    {
        QImage cacheImage(cacheFilePath);
        if(!cacheImage.isNull())
        {
            qDebug() << "[VideoPlayerScreenshot] Using cached image for video file: " << _videoFile;
            emit screenshotReady(cacheImage);
            return;
        }
    }

    // Have to start VLC to grab a new screenshot
    if(!startPlayer())
        emit screenshotReady(QImage());
}

void VideoPlayerScreenshot::handleScreenshot()
{
    QImage* screenshot = getImage();
    qDebug() << "[VideoPlayerScreenshot] Screenshot frame received: " << screenshot;
    emit screenshotReady(screenshot ? (*screenshot) : QImage());

    stopThenDelete();
}
