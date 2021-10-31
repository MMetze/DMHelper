#include "videoplayerglscreenshot.h"
#include "videoplayerglvideo.h"
#include <QImage>
#include <QOpenGLFramebufferObject>
#include <QDebug>

VideoPlayerGLScreenshot::VideoPlayerGLScreenshot(const QString& videoFile, QObject *parent) :
    VideoPlayerGL(parent),
    _videoFile(videoFile),
    _video(nullptr),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr)
{
#ifdef Q_OS_WIN
    _videoFile.replace("/","\\\\");
#endif
}

VideoPlayerGLScreenshot::~VideoPlayerGLScreenshot()
{
    stopPlayer();
    delete _video;
}

void VideoPlayerGLScreenshot::retrieveScreenshot()
{
    if(!initializeVLC())
        emit screenshotReady(QImage());
}

void VideoPlayerGLScreenshot::registerNewFrame()
{
    qDebug() << "[VideoPlayerGLScreenshot] Screenshot frame received";
    emit screenshotReady(extractImage());
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

    return true;
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
