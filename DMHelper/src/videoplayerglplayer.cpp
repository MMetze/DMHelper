#include "videoplayerglplayer.h"
#include "videoplayerglvideo.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>
#include <QDebug>

#define VIDEO_DEBUG_MESSAGES

const int stopCallComplete = 0x01;
const int stopConfirmed = 0x02;
const int stopComplete = stopCallComplete | stopConfirmed;
const int INVALID_TRACK_ID = -99999;

VideoPlayerGLPlayer::VideoPlayerGLPlayer(const QString& videoFile, QOpenGLContext* context, QSurfaceFormat format, QSize targetSize, bool playVideo, bool playAudio, QObject *parent) :
    VideoPlayerGL(parent),
    _videoFile(videoFile),
    _context(context),
    _format(format),
    _videoSize(),
    _playVideo(playVideo),
    _playAudio(playAudio),
    _video(nullptr),
    //_tempTexture(0),
    _fboTexture(-1),
    _vlcError(false),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    //_originalSize(),
    _targetSize(targetSize),
    _status(-1),
    _selfRestart(false),
    _deleteOnStop(false),
    _stopStatus(0),
    _firstImage(false),
    _originalTrack(INVALID_TRACK_ID),
    _modelMatrix(),
    _VAO(0),
    _VBO(0),
    _EBO(0)
{
    if(_context)
    {
#ifdef Q_OS_WIN
        _videoFile.replace("/","\\\\");
#endif
        _vlcError = !initializeVLC();
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayerGLPlayer] Player object initialized: " << this;
#endif

        createGLObjects();
    }
}

VideoPlayerGLPlayer::~VideoPlayerGLPlayer()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Destroying player object: " << this;
#endif

    _selfRestart = false;
    stopPlayer();

    delete _video;

    cleanupGLObjects();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Player object destroyed: " << this;
#endif

}

const QString& VideoPlayerGLPlayer::getFileName() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting file name: " << _videoFile;
#endif

    return _videoFile;
}

/*
QOpenGLFramebufferObject* VideoPlayerGL::getVideoFrame()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting the current video frame.";
#endif

    QMutexLocker locker(&_mutex);
    if(_newImage)
    {
        std::swap(_indexSwap, _indexDisplay);
        _newImage = false;
    }
    return _buffers[_indexDisplay];
}
*/

void VideoPlayerGLPlayer::paintGL()
{
    if((!_context) || (!_video))
        return;

    //    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    //    QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
        return;

    /*
    if(_video->isNewFrameAvailable())
    {
        QOpenGLFramebufferObject *fbo = _video->getVideoFrame();
        if(!fbo)
            return;

        if(_fboTexture > 0)
            f->glDeleteTextures(1, &_fboTexture);

        _fboTexture = fbo->takeTexture();
    }

    if(_fboTexture == 0)
        return;

    // qDebug() << "[VideoPlayerGLPlayer] Painting new texture: " << fboTexture;
    e->glBindVertexArray(_VAO);
    f->glBindTexture(GL_TEXTURE_2D, _fboTexture);
    f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    */


    QOpenGLFramebufferObject *fbo = _video->getVideoFrame();
    if(!fbo)
        return;

    e->glBindVertexArray(_VAO);
    GLuint fboTexture = fbo->takeTexture();
    // qDebug() << "[VideoPlayerGLPlayer] Painting new texture: " << fboTexture;
    if(fboTexture > 0)
    {
        f->glBindTexture(GL_TEXTURE_2D, fboTexture);
        //f->glBindTexture(GL_TEXTURE_2D, _tempTexture);
        f->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        f->glDeleteTextures(1, &fboTexture);
    }
}

bool VideoPlayerGLPlayer::isPlayingVideo() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting playing video state: " << _playVideo;
#endif

    return _playVideo;
}

void VideoPlayerGLPlayer::setPlayingVideo(bool playVideo)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Setting playing video state: " << playVideo;
#endif

    _playVideo = playVideo;
}

bool VideoPlayerGLPlayer::isPlayingAudio() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting playing audio state: " << _playAudio;
#endif

    return _playAudio;
}

void VideoPlayerGLPlayer::setPlayingAudio(bool playAudio)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Setting playing audio state: " << playAudio;
#endif
    _playAudio = playAudio;

    // TBD
    /*
    if(!_vlcPlayer)
        return;

    if(_playAudio)
    {
        if((_originalTrack != INVALID_TRACK_ID) && (_originalTrack != -1))
            libvlc_audio_set_track(_vlcPlayer, _originalTrack);
    }
    else
    {
        _originalTrack = libvlc_audio_get_track(_vlcPlayer);
        if(_originalTrack != -1)
            libvlc_audio_set_track(_vlcPlayer, -1);
    }
    */

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Playing audio state set";
#endif

}

bool VideoPlayerGLPlayer::isError() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting error state: " << _vlcError;
#endif

    return _vlcError;
}

/*
QMutex* VideoPlayerGL::getMutex()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting mutex: " << &_mutex;
#endif

    return &_mutex;
}
*/

QSize VideoPlayerGLPlayer::getOriginalSize() const
{
    QSize originalSize = _video ? _video->getVideoSize() : QSize();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting original size: " << originalSize;
#endif

    return originalSize;
}

/*
bool VideoPlayerGL::isNewImage() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting new image state: " << _newImage;
#endif

    return _newImage;
}

void VideoPlayerGL::clearNewImage()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Clearing new image state";
#endif

    _newImage = false;
}
*/

void VideoPlayerGLPlayer::registerNewFrame()
{
    //qDebug() << "[VideoPlayerGLPlayer] Confirming frame available";
    emit frameAvailable();
}

QSurfaceFormat VideoPlayerGLPlayer::getFormat() const
{
    return _format;
}

QSize VideoPlayerGLPlayer::getSize() const
{
    return _videoSize;
}

QImage VideoPlayerGLPlayer::getLastScreenshot()
{
    if(!_video)
        return QImage();

    QOpenGLFramebufferObject* fbo = _video->getVideoFrame();
    if(!fbo)
        return QImage();

    return fbo->toImage();
}

/*
// this callback will create the surfaces and FBO used by VLC to perform its rendering
bool VideoPlayerGL::resizeRenderTextures(void* data, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *render_cfg)
{
    qDebug() << "[VideoPlayerGLPlayer] resizeRenderTextures";

    VideoPlayerGL* that = static_cast<VideoPlayerGL*>(data);
    if(!that)
        return false;

    if((that->_buffers[0] == nullptr) || (cfg->width != that->_originalSize.width()) || (cfg->height != that->_originalSize.height()))
    {
        cleanup(data);

        that->_buffers[0] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
        that->_buffers[1] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
        that->_buffers[2] = new QOpenGLFramebufferObject(cfg->width, cfg->height);

        that->_originalSize = QSize(static_cast<int>(cfg->width), static_cast<int>(cfg->height));
    }

    that->_buffers[that->_indexRender]->bind();

    render_cfg->opengl_format = GL_RGBA;
    render_cfg->full_range = true;
    render_cfg->colorspace = libvlc_video_colorspace_BT709;
    render_cfg->primaries  = libvlc_video_primaries_BT709;
    render_cfg->transfer   = libvlc_video_transfer_func_SRGB;

    return true;
}

// This callback is called during initialisation.
bool VideoPlayerGL::setup(void** data, const libvlc_video_setup_device_cfg_t *cfg, libvlc_video_setup_device_info_t *out)
{
    qDebug() << "[VideoPlayerGLPlayer] setup";

    Q_UNUSED(cfg);
    Q_UNUSED(out);

    if (!QOpenGLContext::supportsThreadedOpenGL())
        return false;

    VideoPlayerGL* that = static_cast<VideoPlayerGL*>(*data);
    if(!that)
        return false;

    // Wait for rendering view to be ready.
    // TBD - do we need the Semaphore
    //that->videoReady.acquire();

    that->_originalSize = QSize();
    return true;
}

// This callback is called to release the texture and FBO created in resize
void VideoPlayerGL::cleanup(void* data)
{
    qDebug() << "[VideoPlayerGLPlayer] cleanup";

    VideoPlayerGL* that = static_cast<VideoPlayerGL*>(data);
    if(!that)
        return;

    that->cleanupBuffers();
}

//This callback is called after VLC performs drawing calls
void VideoPlayerGL::swap(void* data)
{
    qDebug() << "[VideoPlayerGLPlayer] swap";

    VideoPlayerGL* that = static_cast<VideoPlayerGL*>(data);
    if(!that)
        return;

    QMutexLocker locker(&that->_mutex);
    std::swap(that->_indexSwap, that->_indexRender);
    that->_buffers[that->_indexRender]->bind();
    that->_newImage = true;
    emit that->frameAvailable();

    if(!that->_firstImage)
    {
        that->_firstImage = true;
        emit that->screenShotAvailable();
    }
}

// This callback is called to set the OpenGL context
bool VideoPlayerGL::makeCurrent(void* data, bool current)
{
    VideoPlayerGL* that = static_cast<VideoPlayerGL*>(data);
    if((!that) || (!that->_context))
        return false;

    if (current)
        that->_context->makeCurrent(that->_surface);
    else
        that->_context->doneCurrent();

    return true;
}

// This callback is called by VLC to get OpenGL functions.
void* VideoPlayerGL::getProcAddress(void* data, const char* current)
{
    VideoPlayerGL* that = static_cast<VideoPlayerGL*>(data);
    if((!that) || (!that->_context))
        return nullptr;

    // Qt usual expects core symbols to be queryable, even though it's not
    // mentioned in the API. Cf QPlatformOpenGLContext::getProcAddress.
    // Thus, we don't need to wrap the function symbols here, but it might
    // fail to work (not crash though) on exotic platforms since Qt doesn't
    // commit to this behaviour. A way to handle this in a more stable way
    // would be to store the mContext->functions() table in a thread local
    // variable, and wrap every call to OpenGL in a function using this
    // thread local state to call the correct variant.
    return reinterpret_cast<void*>(that->_context->getProcAddress(current));
}
*/

/**
 * Callback prototype for LibVLC log message handler.
 *
 * \param data data pointer as given to libvlc_log_set()
 * \param level message level (@ref libvlc_log_level)
 * \param ctx message context (meta-information about the message)
 * \param fmt printf() format string (as defined by ISO C11)
 * \param args variable argument list for the format
 * \note Log message handlers <b>must</b> be thread-safe.
 * \warning The message context pointer, the format string parameters and the
 *          variable arguments are only valid until the callback returns.
 */
/*
void VideoPlayerGL::playerLogCallback(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args)
{
    Q_UNUSED(data);
    Q_UNUSED(level);
    Q_UNUSED(ctx);
    Q_UNUSED(fmt);
    Q_UNUSED(args);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] VLC Log: " << QString::vasprintf(fmt, args);
#endif

    return;
}
*/

/*
void* VideoPlayerGL::lockCallback(void **planes)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Lock callback called";
#endif

    _mutex->lock();

    if((planes) && (_nativeBuffer))
    {
        *planes = _nativeBuffer;
    }

    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
        libvlc_clearerr();
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Lock completed";
#endif

    return nullptr;
}

void VideoPlayerGL::unlockCallback(void *picture, void *const *planes)
{
    Q_UNUSED(picture);
    Q_UNUSED(planes);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Unlock callback called. New Image: " << _newImage;
#endif

    if((!_nativeBuffer)  || (_nativeWidth == 0) || (_nativeHeight == 0) || (!_mutex))
        return;

    _newImage = true;

    _mutex->unlock();

    if(!_firstImage)
    {
        _firstImage = true;
        emit screenShotAvailable();
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Unlock completed";
#endif
}

void VideoPlayerGL::displayCallback(void *picture)
{
    Q_UNUSED(picture);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Display callback called";
#endif
}

unsigned VideoPlayerGL::formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Format callback called";
#endif

    if((!chroma)||(!width)||(!height)||(!pitches)||(!lines))
        return 0;

    if(_nativeBufferNotAligned)
        return 0;

    qDebug() << "[VideoPlayer] Format Callback with chroma: " << QString(chroma) << ", width: " << *width << ", height: " << *height << ", pitches: " << *pitches << ", lines: " << *lines;

    //memcpy(chroma, "RV32", sizeof("RV32") - 1);
    memcpy(chroma, "BGRA", sizeof("BGRA") - 1);

    _originalSize = QSize(static_cast<int>(*width), static_cast<int>(*height));
    QSize scaledTarget = _originalSize;

    if((_targetSize.width() > 0) && (_targetSize.height() > 0))
    {
        scaledTarget.scale(_targetSize, Qt::KeepAspectRatio);
        *width = static_cast<unsigned int>(scaledTarget.width());
        *height = static_cast<unsigned int>(scaledTarget.height());
    }

    _nativeWidth = *width;
    _nativeHeight = *height;
    *pitches = (*width) * 4;
    *lines = *height;

    _nativeBufferNotAligned = static_cast<uchar*>(malloc((_nativeWidth * _nativeHeight * 4) + 31));
    _nativeBuffer = reinterpret_cast<uchar*>((size_t(_nativeBufferNotAligned)+31) & static_cast<unsigned long long>(~31));
//    _loadImage = new QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_RGB32);
    _loadImage = new QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_ARGB32);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Format callback completed";
#endif

    return 1;
}

void VideoPlayerGL::cleanupCallback()
{
    qDebug() << "[VideoPlayer] Cleanup Callback";
    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
        libvlc_clearerr();
    }
    cleanupBuffers();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Cleanup callback completed";
#endif
}

void VideoPlayerGL::exitEventCallback()
{
    qDebug() << "[VideoPlayer] Exit Event Callback";
    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
        libvlc_clearerr();
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Exit event callback completed";
#endif
}

void VideoPlayerGL::eventCallback(const struct libvlc_event_t *p_event)
{
    if(p_event)
    {
        switch(p_event->type)
        {
            case libvlc_MediaPlayerOpening:
                qDebug() << "[vlc] Video event received: OPENING = " << p_event->type;
                emit videoOpening();
                break;
            case libvlc_MediaPlayerBuffering:
                qDebug() << "[vlc] Video event received: BUFFERING = " << p_event->type;
                emit videoBuffering();
                break;
            case libvlc_MediaPlayerPlaying:
                qDebug() << "[vlc] Video event received: PLAYING = " << p_event->type;
                internalAudioCheck(p_event->type);
                emit videoPlaying();
                break;
            case libvlc_MediaPlayerPaused:
                qDebug() << "[vlc] Video event received: PAUSED = " << p_event->type;
                emit videoPaused();
                break;
            case libvlc_MediaPlayerStopped:
                qDebug() << "[vlc] Video event received: STOPPED = " << p_event->type;
                internalStopCheck(stopConfirmed);
                emit videoStopped();
                break;
            case libvlc_MediaListPlayerPlayed:
                qDebug() << "[vlc] Video event received: LIST PLAYED = " << p_event->type;
                break;
            case libvlc_MediaListPlayerStopped:
                qDebug() << "[vlc] Video event received: LIST STOPPED = " << p_event->type;
                internalStopCheck(stopConfirmed);
                emit videoStopped();
                break;
            default:
                qDebug() << "[vlc] UNEXPECTED Video event received:  " << p_event->type;
                break;
        };

        _status = p_event->type;
    }
}
*/

void VideoPlayerGLPlayer::targetResized(const QSize& newSize)
{
    qDebug() << "[VideoPlayerGLPlayer] Target window resized: " << newSize;
    _targetSize = newSize;
    videoResized();
    restartPlayer();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Target window resize completed";
#endif

}

void VideoPlayerGLPlayer::stopThenDelete()
{
    if(isProcessing())
    {
        qDebug() << "[VideoPlayerGLPlayer] Stop Then Delete triggered, stop called...";
        _deleteOnStop = true;
        stopPlayer();
    }
    else
    {
        qDebug() << "[VideoPlayerGLPlayer] Stop Then Delete triggered, immediate delete possible.";
        delete this;
    }


#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] stopThenDelete completed";
#endif

}

bool VideoPlayerGLPlayer::restartPlayer()
{
    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayerGLPlayer] Restart Player called, stop called...";
        _selfRestart = true;
        return stopPlayer();
    }
    else
    {
        qDebug() << "[VideoPlayerGLPlayer] Restart Player called, but no player running - starting player!";
        //return false;
        return startPlayer();
    }
}

void VideoPlayerGLPlayer::videoResized()
{
    qDebug() << "[VideoPlayerGLPlayer] Video being resized, recreating vertex arrays";
    cleanupVBObjects();
    createVBObjects();
}

void VideoPlayerGLPlayer::initializationComplete()
{
    if((!_context) || (!_video))
        return;

    qDebug() << "[VideoPlayerGLPlayer] Confirming initialization complete";
    emit contextReady(_context);
}

bool VideoPlayerGLPlayer::initializeVLC()
{
    qDebug() << "[VideoPlayerGLPlayer] Initializing VLC!";

    if(!_context)
    {
        qDebug() << "[VideoPlayerGLPlayer] ERROR: No context provided, not initializing VLC!";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayerGLPlayer] ERROR: Playback file empty - not initializing VLC!";
        return false;
    }

    if(!DMH_VLC::Instance())
        return false;

    // TBD - don't think I need this it seems doubled
    // libvlc_log_set(_vlcInstance, playerLogCallback, nullptr);

    // Use offscreen surface to render the buffers
    //_surface = new QOffscreenSurface(nullptr);
    //_surface->setFormat(format);
    //_surface->create();
    _video = new VideoPlayerGLVideo(this);

    // TBD - do we need this
    //libvlc_set_exit_handler(_vlcInstance, playerExitEventCallback, this);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Initializing VLC completed";
#endif

    //return startPlayer();
    return true;
}

bool VideoPlayerGLPlayer::startPlayer()
{
    if(!DMH_VLC::Instance())
    {
        qDebug() << "[VideoPlayerGLPlayer] VLC not instantiated - not able to start player!";
        return false;
    }

    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayerGLPlayer] Player already running - not able to start player!";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayerGLPlayer] Playback file empty - not able to start player!";
        return false;
    }

    qDebug() << "[VideoPlayerGLPlayer] Starting video player with " << _videoFile.toUtf8().constData();

    // Create a new Media List and add the media to it
    //libvlc_media_list_t *vlcMediaList = libvlc_media_list_new();
    //if (!vlcMediaList)
    //    return false;

    // Create a new Media
    _vlcMedia = libvlc_media_new_path(DMH_VLC::Instance(), _videoFile.toUtf8().constData());
    //https://en.savefrom.net/18/
    //QString ytPath("https://r1---sn-w5nuxa-c33ey.googlevideo.com/videoplayback?expire=1597525099&ei=C_g3X8GwLLHU3LUP6dOm6A4&ip=14.207.129.148&id=o-AKlo5xUHtI-1uAnEPCm0wXnPupzmzuiOIXrUGtmT9WvJ&itag=22&source=youtube&requiressl=yes&mh=3O&mm=31%2C26&mn=sn-w5nuxa-c33ey%2Csn-npoe7ne6&ms=au%2Conr&mv=m&mvi=1&pl=23&initcwndbps=812500&vprv=1&mime=video%2Fmp4&ratebypass=yes&dur=167.090&lmt=1597239028450972&mt=1597503397&fvip=1&fexp=23883098&c=WEB&txp=6316222&sparams=expire%2Cei%2Cip%2Cid%2Citag%2Csource%2Crequiressl%2Cvprv%2Cmime%2Cratebypass%2Cdur%2Clmt&sig=AOq0QJ8wRQIgHwkUXh_YN2OS5o76bNa1APrbw3G4nMZgjVQQhMj7OUoCIQDesCxcrVOBSme7QNmar0mkG5U8fz_01LP3CAoXpmCwaQ%3D%3D&lsparams=mh%2Cmm%2Cmn%2Cms%2Cmv%2Cmvi%2Cpl%2Cinitcwndbps&lsig=AG3C_xAwRQIhAKjExXaqpMXxMk4sOFBoQBg6c7kfVKYnhFkv43RqJZ0JAiA10pSSMS4ozj73yfIXjEmcLEnqi5sqMEj9EvWTa3EVgg%3D%3D&contentlength=15083894&video_id=9bMTK0ml9ZI&title=%F0%9F%8E%B5+RPG+Boss+Battle+Music+-+Hydra");
    //libvlc_media_t *vlcMedia = libvlc_media_new_location(_vlcInstance, ytPath.toUtf8().constData());
    if (!_vlcMedia)
        return false;

    //libvlc_media_list_add_media(vlcMediaList, vlcMedia);
    //libvlc_media_release(vlcMedia);

    // Create a new libvlc player
    //_vlcListPlayer = libvlc_media_list_player_new(_vlcInstance);
    //if(!_vlcListPlayer)
    //    return false;

    //_vlcPlayer = libvlc_media_player_new(_vlcInstance);
    _vlcPlayer = libvlc_media_player_new_from_media(_vlcMedia);
    if(!_vlcPlayer)
        return false;

    //libvlc_media_list_player_set_media_list(_vlcListPlayer, vlcMediaList);
    //libvlc_media_list_player_set_media_player(_vlcListPlayer, player);
    //libvlc_media_list_player_set_playback_mode(_vlcListPlayer, libvlc_playback_mode_loop);
    libvlc_audio_set_volume(_vlcPlayer, 0);

    //libvlc_video_set_scale(_vlcPlayer, 0.25f );

    // TBD
    /*
    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, playerEventCallback, static_cast<void*>(this));
    }
    */

    /*
    libvlc_event_manager_t* listEventManager = libvlc_media_list_player_event_manager(_vlcListPlayer);
    if(eventManager)
    {
        libvlc_event_attach(listEventManager, libvlc_MediaListPlayerPlayed, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(listEventManager, libvlc_MediaListPlayerStopped, playerEventCallback, static_cast<void*>(this));
    }
    */

    bool callbackResult = libvlc_video_set_output_callbacks(_vlcPlayer,
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

    qDebug() << "[VideoPlayerGLPlayer] Player callback result: " << callbackResult;

    //libvlc_video_set_callbacks(player, playerLockCallback, playerUnlockCallback, playerDisplayCallback, static_cast<void*>(this));
    //libvlc_video_set_format_callbacks(player, playerFormatCallback, playerCleanupCallback);

    // And start playback
    libvlc_media_player_play(_vlcPlayer);
    //libvlc_media_list_player_play(_vlcListPlayer);

    qDebug() << "[VideoPlayerGLPlayer] Player started";

    return true;
}

bool VideoPlayerGLPlayer::stopPlayer()
{
    qDebug() << "[VideoPlayerGLPlayer] Stop Player called";

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

    if(_selfRestart)
    {
        _selfRestart = false;
        startPlayer();
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: player restarted.";
    }

    if(_deleteOnStop)
    {
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: video player being destroyed.";

        // TBD - how to do this...
        //delete this;
    }

    return true;
}

/*
void VideoPlayerGL::cleanupBuffers()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Cleaning up buffers";
#endif

    _newImage = false;
    _originalSize = QSize();

    // TBD - do we need the semaphore?
    //that->videoReady.release();

    delete _buffers[0]; _buffers[0] = nullptr;
    delete _buffers[1]; _buffers[1] = nullptr;
    delete _buffers[2]; _buffers[2] = nullptr;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Buffer cleanup completed";
#endif

}
*/

// TBD - do we need this mechanism?
/*
void VideoPlayerGL::internalStopCheck(int status)
{
    _stopStatus |= status;

    qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check called with status " << status << ", overall status: " << _stopStatus;

    if(_stopStatus != stopComplete)
        return;

    if(_vlcMedia)
    {
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: vlc media released.";
    }

    if(_vlcPlayer)
    {
        cleanupBuffers();
        _vlcPlayer = nullptr;
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: vlc player destroyed.";
    }

    if(_selfRestart)
    {
        _selfRestart = false;
        startPlayer();
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: player restarted.";
    }

    if(_deleteOnStop)
    {
        qDebug() << "[VideoPlayerGLPlayer] Internal Stop Check: video player being destroyed.";
        delete this;
        return;
    }
}
*/

void VideoPlayerGLPlayer::createGLObjects()
{
    if(!_context)
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    //QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    //QOpenGLExtraFunctions *e = QOpenGLContext::currentContext()->extraFunctions();
    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
        return;

    QImage image;
    image.load(QString("C:\\Users\\turne\\Documents\\DnD\\DM Helper\\testdata\\Desert Stronghold.jpg"));

    createVBObjects();

    /*
    // Texture
    f->glGenTextures(1, &_tempTexture);
    f->glBindTexture(GL_TEXTURE_2D, _tempTexture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    f->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    */

    // load and generate the background texture
    QImage glBackgroundImage = image.convertToFormat(QImage::Format_RGBA8888).mirrored();
    f->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, glBackgroundImage.width(), glBackgroundImage.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, glBackgroundImage.bits());
    f->glGenerateMipmap(GL_TEXTURE_2D);

}

void VideoPlayerGLPlayer::cleanupGLObjects()
{
    cleanupVBObjects();
}

void VideoPlayerGLPlayer::createVBObjects()
{
    if((!_context) || (!_video))
        return;

    // Set up the rendering context, load shaders and other resources, etc.:
    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
        return;

    //QSize videoSize = getOriginalSize();
    _videoSize = _video->getVideoSize().scaled(_targetSize, Qt::KeepAspectRatio);
    //QSize videoSize = image.size();
    if((_videoSize.width() <= 0) || (_videoSize.height() <= 0))
        return;

    float vertices[] = {
        // positions    // colors           // texture coords
         (float)_videoSize.width() / 2,  (float)_videoSize.height() / 2, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
         (float)_videoSize.width() / 2, -(float)_videoSize.height() / 2, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
        -(float)_videoSize.width() / 2, -(float)_videoSize.height() / 2, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
        -(float)_videoSize.width() / 2,  (float)_videoSize.height() / 2, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
    };

    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };

    e->glGenVertexArrays(1, &_VAO);
    f->glGenBuffers(1, &_VBO);
    f->glGenBuffers(1, &_EBO);

    e->glBindVertexArray(_VAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);
    f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    f->glEnableVertexAttribArray(0);
    // color attribute
    f->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    f->glEnableVertexAttribArray(1);
    // texture attribute
    f->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    f->glEnableVertexAttribArray(2);
}

void VideoPlayerGLPlayer::cleanupVBObjects()
{
    if(!_context)
        return;

    QOpenGLFunctions *f = _context->functions();
    QOpenGLExtraFunctions *e = _context->extraFunctions();
    if((!f) || (!e))
        return;

    _videoSize = QSize();

    if(_VAO > 0)
    {
        e->glDeleteVertexArrays(1, &_VAO);
        _VAO = 0;
    }

    if(_VBO > 0)
    {
        f->glDeleteBuffers(1, &_VBO);
        _VBO = 0;
    }

    if(_EBO > 0)
    {
        f->glDeleteBuffers(1, &_EBO);
        _EBO = 0;
    }
}

void VideoPlayerGLPlayer::internalAudioCheck(int newStatus)
{
    if((_playAudio) ||
       (_originalTrack != INVALID_TRACK_ID) ||
       (newStatus != libvlc_MediaPlayerPlaying) ||
       (!_vlcPlayer))
        return;

    qDebug() << "[VideoPlayerGLPlayer] Internal Audio Check identified audio, shall be turned off";

    //libvlc_media_player_t * player = libvlc_media_list_player_get_media_player(_vlcListPlayer);
    _originalTrack = libvlc_audio_get_track(_vlcPlayer);
    if(_originalTrack != -1)
        libvlc_audio_set_track(_vlcPlayer, -1);

    qDebug() << "[VideoPlayerGLPlayer] Audio turning off completed";
}

bool VideoPlayerGLPlayer::isPlaying() const
{
    bool result = ((_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is playing status: " << result;
#endif

    return result;
}

bool VideoPlayerGLPlayer::isPaused() const
{
    bool result = (_status == libvlc_MediaPlayerPaused);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is paused status: " << result;
#endif

    return result;
}

bool VideoPlayerGLPlayer::isProcessing() const
{
    bool result = ((_status == libvlc_MediaPlayerOpening) ||
                   (_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying) ||
                   (_status == libvlc_MediaPlayerPaused));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is processing status: " << result;
#endif

    return result;
}

bool VideoPlayerGLPlayer::isStatusValid() const
{
    bool result = ((_status == libvlc_MediaPlayerOpening) ||
                   (_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying) ||
                   (_status == libvlc_MediaPlayerPaused) ||
                   (_status == libvlc_MediaPlayerStopped));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLPlayer] Getting is status valid: " << result;
#endif

    return result;
}





