#include "videoplayerglscreenshot.h"
#include "videoplayerglvideo.h"
#include "dmhcache.h"
#include <QImage>
#include <QImageReader>
#include <QFile>
#include <QOpenGLFramebufferObject>
#include <QTimerEvent>
#include <QDebug>

const int SCREENSHOT_USE_FRAME = VIDEO_BUFFER_COUNT + 1;

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

void VideoPlayerGLScreenshot::playerEventCallback(const struct libvlc_event_t *p_event, void *p_data)
{
    if((!p_event) || (!p_data))
        return;

    VideoPlayerGLScreenshot* that = static_cast<VideoPlayerGLScreenshot*>(p_data);
    if(!that)
        return;

    switch(p_event->type)
    {
        case libvlc_MediaPlayerOpening:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: OPENING = " << p_event->type;
            break;
        case libvlc_MediaPlayerBuffering:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: BUFFERING = " << p_event->type;
            break;
        case libvlc_MediaPlayerPlaying:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: PLAYING = " << p_event->type;
            break;
        case libvlc_MediaPlayerPaused:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: PAUSED = " << p_event->type;
            break;
        case libvlc_MediaPlayerStopped:
            qDebug() << "[VideoPlayerGLScreenshot] Video event received: STOPPED = " << p_event->type;
            break;
        default:
            qDebug() << "[VideoPlayerGLScreenshot] UNEXPECTED Video event received:  " << p_event->type;
            break;
    };

    that->_status = p_event->type;
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
    if((_status == libvlc_MediaPlayerOpening) || (_status == libvlc_MediaPlayerBuffering) || (_status == libvlc_MediaPlayerPlaying))
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
#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)
    _vlcMedia = libvlc_media_new_path(localizedVideoFile.toUtf8().constData());
#else
    _vlcMedia = libvlc_media_new_path(DMH_VLC::vlcInstance(), localizedVideoFile.toUtf8().constData());
#endif
    if(!_vlcMedia)
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, unable to open video file!";
        return false;
    }

#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)
    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), _vlcMedia);
#else
    _vlcPlayer = libvlc_media_player_new_from_media(_vlcMedia);
#endif
    if(!_vlcPlayer)
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, unable to start media player";
        return false;
    }

    // Set up event callbacks
    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, playerEventCallback, static_cast<void*>(this));
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
