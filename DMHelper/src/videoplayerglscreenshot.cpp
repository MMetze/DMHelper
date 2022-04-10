#include "videoplayerglscreenshot.h"
#include "videoplayerglvideo.h"
#include <QImage>
#include <QOpenGLFramebufferObject>
#include <QTimerEvent>
#include <QDebug>

const int SCREENSHOT_USE_FRAME = 3;

VideoPlayerGLScreenshot::VideoPlayerGLScreenshot(const QString& videoFile, QObject *parent) :
    VideoPlayerGL(parent),
    _videoFile(videoFile),
    _video(nullptr),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    _framesReceived(0),
    _status(-1)
{
#ifdef Q_OS_WIN
    _videoFile.replace("/","\\\\");
#endif
}

VideoPlayerGLScreenshot::~VideoPlayerGLScreenshot()
{
    cleanupPlayer();
}

void VideoPlayerGLScreenshot::retrieveScreenshot()
{
    if(!initializeVLC())
        emit screenshotReady(QImage());
}

void VideoPlayerGLScreenshot::registerNewFrame()
{
    if(_framesReceived >= SCREENSHOT_USE_FRAME)
        return;

    ++_framesReceived;
    qDebug() << "[VideoPlayerGLScreenshot] Screenshot frame received, #" << _framesReceived << " from " << SCREENSHOT_USE_FRAME;

    if(_framesReceived >= SCREENSHOT_USE_FRAME)
    {
        emit screenshotReady(extractImage());
        stopPlayer();
    }
}

void VideoPlayerGLScreenshot::playerEventCallback( const struct libvlc_event_t *p_event, void *p_data )
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

void VideoPlayerGLScreenshot::timerEvent(QTimerEvent *event)
{
    if((_status == libvlc_MediaPlayerOpening) || (_status == libvlc_MediaPlayerBuffering) || (_status == libvlc_MediaPlayerPlaying))
        return;

    cleanupPlayer();
    killTimer(event->timerId());
    deleteLater();
}

bool VideoPlayerGLScreenshot::initializeVLC()
{
    if(_videoFile.isEmpty())
        return false;

    if(!DMH_VLC::Instance())
        return false;

    _video = new VideoPlayerGLVideo(this);

    return startPlayer();
}

bool VideoPlayerGLScreenshot::startPlayer()
{
    if((!DMH_VLC::Instance()) || (_vlcPlayer))
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, already running";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Playback file empty - not able to start player!";
        return false;
    }

    _vlcMedia = libvlc_media_new_path(DMH_VLC::Instance(), _videoFile.toUtf8().constData());
    if (!_vlcMedia)
    {
        qDebug() << "[VideoPlayerGLScreenshot] ERROR: Can't start screenshot grabber, unable to open video file!";
        return false;
    }

    _vlcPlayer = libvlc_media_player_new_from_media(_vlcMedia);
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

    qDebug() << "[VideoPlayerGLScreenshot] Playback started to get screenshot for " << _videoFile;

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

    return true;
}

bool VideoPlayerGLScreenshot::stopPlayer()
{
    _framesReceived = SCREENSHOT_USE_FRAME;

    if(_vlcPlayer)
        libvlc_media_player_stop_async(_vlcPlayer);

    startTimer(500);

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

    if(_video)
    {
        delete _video;
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
