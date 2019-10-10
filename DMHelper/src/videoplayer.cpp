#include "videoplayer.h"
#include <QDebug>

//#define VIDEO_DEBUG_MESSAGES

// libvlc callback static functions
void * playerLockCallback(void *opaque, void **planes);
void playerUnlockCallback(void *opaque, void *picture, void *const *planes);
void playerDisplayCallback(void *opaque, void *picture);
unsigned playerFormatCallback(void **opaque, char *chroma,
                              unsigned *width, unsigned *height,
                              unsigned *pitches,
                              unsigned *lines);
void playerCleanupCallback(void *opaque);
void playerExitEventCallback(void *opaque);
void playerLogCallback(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args);
void playerEventCallback(const struct libvlc_event_t *p_event, void *p_data);


VideoPlayer::VideoPlayer(QSize targetSize, QObject *parent) :
    QObject(parent),
    _vlcError(false),
    _vlcInstance(nullptr),
    _vlcListPlayer(nullptr),
    _nativeWidth(0),
    _nativeHeight(0),
    _nativeBufferNotAligned(nullptr),
    _nativeBuffer(nullptr),
    _mutex(new QMutex(QMutex::Recursive)),
    _loadImage(nullptr),
    _newImage(false),
    _targetSize(targetSize),
    _status(-1)
{
    _vlcError = !initializeVLC();
}

VideoPlayer::~VideoPlayer()
{
    if(_vlcListPlayer)
    {
        libvlc_media_list_player_stop(_vlcListPlayer);
        libvlc_media_list_player_release(_vlcListPlayer);
    }

    cleanupBuffers();

    if(_vlcInstance)
        libvlc_release(_vlcInstance);

    delete _mutex;
}

bool VideoPlayer::isError() const
{
    return _vlcError;
}

QMutex* VideoPlayer::getMutex() const
{
    return _mutex;
}

QImage* VideoPlayer::getImage() const
{
    if((_status == libvlc_MediaPlayerBuffering) || (_status == libvlc_MediaPlayerPlaying))
        return _loadImage;
    else
        return nullptr;
}

bool VideoPlayer::isNewImage() const
{
    return _newImage;
}

void VideoPlayer::clearNewImage()
{
    _newImage = false;
}

void* VideoPlayer::lockCallback(void **planes)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Lock callback called";
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

    return nullptr;
}

void VideoPlayer::unlockCallback(void *picture, void *const *planes)
{
    Q_UNUSED(picture);
    Q_UNUSED(planes);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Unlock callback called. New Image: " << _newImage;
#endif

    if((!_nativeBuffer)  || (_nativeWidth == 0) || (_nativeHeight == 0) || (!_mutex))
        return;

    /*
    if(!_newImage)
    {
        // QImage::Format_RGB32 works fine as well and it's much faster
        _loadImage = QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_RGB32);
        _newImage = true;
    }
    */
    _newImage = true;

    _mutex->unlock();
}

void VideoPlayer::displayCallback(void *picture)
{
    Q_UNUSED(picture);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Display callback called";
#endif
}

unsigned VideoPlayer::formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    if((!chroma)||(!width)||(!height)||(!pitches)||(!lines))
        return 0;

    if(_nativeBufferNotAligned)
        return 0;

    qDebug() << "[VideoPlayer] Format Callback with chroma: " << QString(chroma) << ", width: " << *width << ", height: " << *height << ", pitches: " << *pitches << ", lines: " << *lines;

    memcpy(chroma, "RV32", sizeof("RV32") - 1);

    QSize scaledTarget(static_cast<int>(*width), static_cast<int>(*height));

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
    _loadImage = new QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_RGB32);

    return 1;
}

void VideoPlayer::cleanupCallback()
{
    qDebug() << "[VideoPlayer] Cleanup Callback";
    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
        libvlc_clearerr();
    }
    cleanupBuffers();
}

void VideoPlayer::exitEventCallback()
{
    qDebug() << "[VideoPlayer] Exit Event Callback";
    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
        libvlc_clearerr();
    }
}

void VideoPlayer::eventCallback(const struct libvlc_event_t *p_event)
{
    if(p_event)
    {
        if((p_event->type == libvlc_MediaPlayerOpening) ||
           (p_event->type == libvlc_MediaPlayerBuffering) ||
           (p_event->type == libvlc_MediaPlayerPlaying) ||
           (p_event->type == libvlc_MediaPlayerPaused) ||
           (p_event->type == libvlc_MediaPlayerStopped))
        {
            qDebug() << "[vlc] Video event received:  " << p_event->type;
            _status = p_event->type;
        }
        else
        {
            qDebug() << "[vlc] UNEXPECTED Video event received:  " << p_event->type;
            _status = -1;
        }
    }
}

void VideoPlayer::targetResized(const QSize& newSize)
{
    qDebug() << "[VideoPlayer] Target window resized: " << newSize;
    _targetSize = newSize;
    /*
    if((_vlcInstance) && (_vlcListPlayer) && (!_vlcError))
    {
        libvlc_media_player_t* player = libvlc_media_list_player_get_media_player(_vlcListPlayer);
        if(player)
        {
            //libvlc_media_list_player_pause(_vlcListPlayer);
            libvlc_video_set_format_callbacks(player, playerFormatCallback, playerCleanupCallback);
            //libvlc_media_list_player_pause(_vlcListPlayer);
        }
    }
    */
}

bool VideoPlayer::initializeVLC()
{
    qDebug() << "[VideoPlayer] Initializing VLC!";
    _vlcInstance = libvlc_new(0, nullptr);

    if(!_vlcInstance)
        return false;

    libvlc_log_set(_vlcInstance, playerLogCallback, nullptr);

    QString fileOpen = QString("C:\\Users\\Craig\\Documents\\Dnd\\Animated Maps\\Airship_gridLN.m4v");

    // Stop if something is playing
    //if(_vlcListPlayer && libvlc_media_list_player_is_playing(vlcListPlayer))
    //    return;

    libvlc_set_exit_handler(_vlcInstance, playerExitEventCallback, this);

    // Create a new Media List and add the media to it
    libvlc_media_list_t *vlcMediaList = libvlc_media_list_new(_vlcInstance);
    if (!vlcMediaList)
        return false;

    // Create a new Media
    libvlc_media_t *vlcMedia = libvlc_media_new_path(_vlcInstance, fileOpen.toUtf8().constData());
    if (!vlcMedia)
        return false;

    libvlc_media_list_add_media(vlcMediaList, vlcMedia);
    libvlc_media_release(vlcMedia);

    // Create a new libvlc player
    _vlcListPlayer = libvlc_media_list_player_new(_vlcInstance);
    libvlc_media_player_t *player = libvlc_media_player_new(_vlcInstance);

    libvlc_media_list_player_set_media_list(_vlcListPlayer, vlcMediaList);
    libvlc_media_list_player_set_media_player(_vlcListPlayer, player);
    libvlc_media_list_player_set_playback_mode(_vlcListPlayer, libvlc_playback_mode_loop);
    libvlc_audio_set_track(player, -1);
    libvlc_video_set_scale(player, 0.25f );
    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(player);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, playerEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, playerEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, playerEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, playerEventCallback, this);
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, playerEventCallback, this);
    }

    unsigned x = 0;
    unsigned y = 0;
    int result = libvlc_video_get_size(player, 0, &x, &y);
    qDebug() << "[VideoPlayer] Video size (result: " << result << "): " << x << " x " << y << ". File: " << fileOpen;

    libvlc_video_set_callbacks(player, playerLockCallback, playerUnlockCallback, playerDisplayCallback, this);
    libvlc_video_set_format_callbacks(player, playerFormatCallback, playerCleanupCallback);
    //libvlc_video_set_format(player, "RV32", _nativeWidth, _nativeHeight, _nativeWidth*4);

    // And start playback
    libvlc_media_list_player_play(_vlcListPlayer);

    return true;
}

void VideoPlayer::cleanupBuffers()
{
    _newImage = false;

    if(_loadImage)
    {
        QImage* tempImage = _loadImage;
        _loadImage = nullptr;
        delete tempImage;
    }

    if(_nativeBufferNotAligned)
    {
        unsigned char* tempChar = _nativeBufferNotAligned;
        _nativeBufferNotAligned = nullptr;
        _nativeBuffer = nullptr;
        free(tempChar);
    }
}


// libvlc callback static functions
/**
 * Callback prototype to allocate and lock a picture buffer.
 *
 * Whenever a new video frame needs to be decoded, the lock callback is
 * invoked. Depending on the video chroma, one or three pixel planes of
 * adequate dimensions must be returned via the second parameter. Those
 * planes must be aligned on 32-bytes boundaries.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks() [IN]
 * \param planes start address of the pixel planes (LibVLC allocates the array
 *             of void pointers, this callback must initialize the array) [OUT]
 * \return a private pointer for the display and unlock callbacks to identify
 *         the picture buffers
 */
void * playerLockCallback(void *opaque, void **planes)
{
    if(!opaque)
        return nullptr;

    VideoPlayer* player = static_cast<VideoPlayer*>(opaque);
    return player->lockCallback(planes);
}

/**
 * Callback prototype to unlock a picture buffer.
 *
 * When the video frame decoding is complete, the unlock callback is invoked.
 * This callback might not be needed at all. It is only an indication that the
 * application can now read the pixel values if it needs to.
 *
 * \note A picture buffer is unlocked after the picture is decoded,
 * but before the picture is displayed.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks() [IN]
 * \param picture private pointer returned from the @ref libvlc_video_lock_cb
 *                callback [IN]
 * \param planes pixel planes as defined by the @ref libvlc_video_lock_cb
 *               callback (this parameter is only for convenience) [IN]
 */
void playerUnlockCallback(void *opaque, void *picture, void *const *planes)
{
    if(!opaque)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(opaque);
    player->unlockCallback(picture, planes);
}

/**
 * Callback prototype to display a picture.
 *
 * When the video frame needs to be shown, as determined by the media playback
 * clock, the display callback is invoked.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks() [IN]
 * \param picture private pointer returned from the @ref libvlc_video_lock_cb
 *                callback [IN]
 */
void playerDisplayCallback(void *opaque, void *picture)
{
    if(!opaque)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(opaque);
    player->displayCallback(picture);
}

/**
 * Callback prototype to configure picture buffers format.
 * This callback gets the format of the video as output by the video decoder
 * and the chain of video filters (if any). It can opt to change any parameter
 * as it needs. In that case, LibVLC will attempt to convert the video format
 * (rescaling and chroma conversion) but these operations can be CPU intensive.
 *
 * \param opaque pointer to the private pointer passed to
 *               libvlc_video_set_callbacks() [IN/OUT]
 * \param chroma pointer to the 4 bytes video format identifier [IN/OUT]
 * \param width pointer to the pixel width [IN/OUT]
 * \param height pointer to the pixel height [IN/OUT]
 * \param pitches table of scanline pitches in bytes for each pixel plane
 *                (the table is allocated by LibVLC) [OUT]
 * \param lines table of scanlines count for each plane [OUT]
 * \return the number of picture buffers allocated, 0 indicates failure
 *
 * \note
 * For each pixels plane, the scanline pitch must be bigger than or equal to
 * the number of bytes per pixel multiplied by the pixel width.
 * Similarly, the number of scanlines must be bigger than of equal to
 * the pixel height.
 * Furthermore, we recommend that pitches and lines be multiple of 32
 * to not break assumptions that might be held by optimized code
 * in the video decoders, video filters and/or video converters.
 */
unsigned playerFormatCallback(void **opaque, char *chroma,
                              unsigned *width, unsigned *height,
                              unsigned *pitches,
                              unsigned *lines)
{
    if((!opaque)||(!(*opaque)))
        return 0;

    VideoPlayer* player = static_cast<VideoPlayer*>(*opaque);
    return player->formatCallback(chroma, width, height, pitches, lines);
}

/**
 * Callback prototype to configure picture buffers format.
 *
 * \param opaque private pointer as passed to libvlc_video_set_callbacks()
 *               (and possibly modified by @ref libvlc_video_format_cb) [IN]
 */
void playerCleanupCallback(void *opaque)
{
    if(!opaque)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(opaque);
    player->cleanupCallback();
}

/**
 * Registers a callback for the LibVLC exit event. This is mostly useful if
 * the VLC playlist and/or at least one interface are started with
 * libvlc_playlist_play() or libvlc_add_intf() respectively.
 * Typically, this function will wake up your application main loop (from
 * another thread).
 *
 * \note This function should be called before the playlist or interface are
 * started. Otherwise, there is a small race condition: the exit event could
 * be raised before the handler is registered.
 *
 * \param p_instance LibVLC instance
 * \param cb callback to invoke when LibVLC wants to exit,
 *           or NULL to disable the exit handler (as by default)
 * \param opaque data pointer for the callback
 * \warning This function and libvlc_wait() cannot be used at the same time.
 */
void playerExitEventCallback(void *opaque)
{
    if(!opaque)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(opaque);
    player->exitEventCallback();
}

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
void playerLogCallback(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args)
{
    Q_UNUSED(data);
    Q_UNUSED(level);
    Q_UNUSED(ctx);
    Q_UNUSED(fmt);
    Q_UNUSED(args);
    //qDebug() << "[vlc] " << QString::vasprintf(fmt, args);
    return;
}

/**
 * Callback function notification
 * \param p_event the event triggering the callback
 */
void playerEventCallback(const struct libvlc_event_t *p_event, void *p_data)
{
    if(!p_data)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(p_data);
    player->eventCallback(p_event);
}
