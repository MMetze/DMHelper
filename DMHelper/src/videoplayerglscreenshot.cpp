#include "videoplayerglscreenshot.h"
#include "videoplayerglvideo.h"
#include <vlc/libvlc_version.h>
#include "dmhcache.h"
#include <QImage>
#include <QImageReader>
#include <QFile>
#include <QOpenGLFramebufferObject>
#include <QTimerEvent>
#include <QDebug>

const int SCREENSHOT_USE_FRAME = VIDEO_BUFFER_COUNT + 1;

// cbs struct has static storage duration and carries no per-instance state; the opaque pointer supplies the instance
static const libvlc_media_player_cbs s_videoPlayerGLScreenshotCbs = []() {
    libvlc_media_player_cbs cbs{};
    cbs.on_state_changed = &VideoPlayerGLScreenshot::onStateChanged;
    return cbs;
}();

VideoPlayerGLScreenshot::VideoPlayerGLScreenshot(const QString& videoFile, QObject *parent) :
    VideoPlayerGL(parent),
    _videoFile(videoFile),
    _video(nullptr),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    _framesReceived(0),
    _status(-1)
{
}

VideoPlayerGLScreenshot::~VideoPlayerGLScreenshot()
{
    cleanupPlayer();
}

void VideoPlayerGLScreenshot::retrieveScreenshot()
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
            qDebug() << "[VideoPlayerGLScreenshot] Using cached image for video file: " << _videoFile;
            emit screenshotReady(cacheImage);
            return;
        }
    }

    // Have to start VLC to grab a new screenshot
    if(!initializeVLC())
        emit screenshotReady(QImage());
}

void VideoPlayerGLScreenshot::registerNewFrame()
{
    if(_framesReceived >= SCREENSHOT_USE_FRAME)
        return;

    ++_framesReceived;
    qDebug() << "[VideoPlayerGLScreenshot] Screenshot frame received, #" << _framesReceived << " from " << SCREENSHOT_USE_FRAME;

    QImage frameImage = extractImage();
    if(_framesReceived >= SCREENSHOT_USE_FRAME)
    {
        // Try to add the screenshot to the cache
        QString cacheFilePath = DMHCache().getCacheFilePath(_videoFile, QString("png"));
        if((!cacheFilePath.isEmpty()) && (!QFile::exists(cacheFilePath)))
            frameImage.save(cacheFilePath);

        emit screenshotReady(frameImage);
        stopPlayer(false);
    }
}

void VideoPlayerGLScreenshot::onStateChanged(void *opaque, libvlc_state_t state)
{
    if(!opaque)
        return;

    VideoPlayerGLScreenshot* that = static_cast<VideoPlayerGLScreenshot*>(opaque);
    if(!that)
        return;

    switch(state)
    {
        case libvlc_Opening:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: OPENING = " << state;
            break;
        case libvlc_Playing:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: PLAYING = " << state;
            break;
        case libvlc_Paused:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: PAUSED = " << state;
            break;
        case libvlc_Stopped:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: STOPPED = " << state;
            break;
        default:
            qDebug() << "[VideoPlayerGLScreenshot] UNEXPECTED Video event received:  " << state;
            break;
    };

    that->_status = state;
}

void VideoPlayerGLScreenshot::videoAvailable()
{
    if(_video)
        return;

    DMH_VLC *vlcInstance = DMH_VLC::DMH_VLCInstance();
    if(!vlcInstance)
        return;

    _video = vlcInstance->requestVideo(this);
    if(_video)
    {
        qDebug() << "[VideoPlayerGLScreenshot] Video player received: " << _video;
        disconnect(vlcInstance, &DMH_VLC::playerAvailable, this, &VideoPlayerGLScreenshot::videoAvailable);
        if(!startPlayer())
            qDebug() << "[VideoPlayerGLScreenshot] ERROR: Not able to start video: " << _video;
    }

    /*
    _video = new VideoPlayerGLVideo(this);

    return startPlayer();
    */

}

void VideoPlayerGLScreenshot::timerEvent(QTimerEvent *event)
{
    if((_status == libvlc_Opening) || (_status == libvlc_Playing))
        return;

    killTimer(event->timerId());
    cleanupPlayer();
    deleteLater();
}

bool VideoPlayerGLScreenshot::initializeVLC()
{
    if(_videoFile.isEmpty())
        return false;

    DMH_VLC *vlcInstance = DMH_VLC::DMH_VLCInstance();
    if(!vlcInstance)
        return false;

    connect(vlcInstance, &DMH_VLC::playerAvailable, this, &VideoPlayerGLScreenshot::videoAvailable);
    videoAvailable();

    return true;

    /*
    _video = new VideoPlayerGLVideo(this);

    return startPlayer();
    */
}

bool VideoPlayerGLScreenshot::startPlayer()
{
    if((!DMH_VLC::vlcInstance()) || (_vlcPlayer))
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, already running";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Playback file empty - not able to start player!";
        return false;
    }

    QString localizedVideoFile = _videoFile;
#ifdef Q_OS_WIN
    localizedVideoFile.replace("/", "\\\\");
#endif
    _vlcMedia = libvlc_media_new_path(localizedVideoFile.toUtf8().constData());
    if(!_vlcMedia)
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, unable to open video file!";
        return false;
    }

    libvlc_media_add_option(_vlcMedia, ":avcodec-threads=0");

    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), _vlcMedia, &s_videoPlayerGLScreenshotCbs, static_cast<void*>(this));
    if(!_vlcPlayer)
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, unable to start media player";
        return false;
    }

    qDebug() << "[VideoPlayerGLScreenshot] Playback started to get screenshot for " << localizedVideoFile;

    libvlc_audio_set_volume(_vlcPlayer, 0);

    libvlc_video_set_output_callbacks(_vlcPlayer,
                                      libvlc_video_engine_opengl,
                                      VideoPlayerGLVideo::setup,
                                      VideoPlayerGLVideo::cleanup,
                                      nullptr,
                                      VideoPlayerGLVideo::resizeRenderTextures,
                                      VideoPlayerGLVideo::swap,
                                      VideoPlayerGLVideo::makeCurrent,
                                      VideoPlayerGLVideo::getProcAddress,
                                      nullptr,
                                      nullptr,
                                      _video);

    libvlc_media_player_play(_vlcPlayer);
    emit contextReady(nullptr);

    startTimer(500);

    return true;
}

bool VideoPlayerGLScreenshot::stopPlayer(bool restart)
{
    Q_UNUSED(restart);

    _framesReceived = SCREENSHOT_USE_FRAME;

    if(_vlcPlayer)
        libvlc_media_player_stop_async(_vlcPlayer);

    return true;
}

void VideoPlayerGLScreenshot::cleanupPlayer()
{
    if(_vlcPlayer)
    {
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
    }

    if(_vlcMedia)
    {
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
    }

    /*
    if(_video)
    {
        delete _video;
        _video = nullptr;
    }
    */
    DMH_VLC *vlcInstance = DMH_VLC::DMH_VLCInstance();
    if((vlcInstance) && (_video))
    {
        vlcInstance->releaseVideo(_video);
        _video = nullptr;
    }
}

QImage VideoPlayerGLScreenshot::extractImage()
{
    if(!_video)
        return QImage();

    QOpenGLFramebufferObject* fbo = _video->getVideoFrame();
    if(!fbo)
        return QImage();

    return fbo->toImage();
}
