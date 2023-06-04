#include "videoplayer.h"
#include <QDebug>

#define VIDEO_DEBUG_MESSAGES

const int stopCallComplete = 0x01;
const int stopConfirmed = 0x02;
const int stopComplete = stopCallComplete | stopConfirmed;
const int INVALID_TRACK_ID = -99999;

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


VideoPlayer::VideoPlayer(const QString& videoFile, QSize targetSize, bool playVideo, bool playAudio, QObject *parent) :
    QObject(parent),
    _videoFile(videoFile),
    _playVideo(playVideo),
    _playAudio(playAudio),
    _vlcError(false),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    _nativeWidth(0),
    _nativeHeight(0),
    _mutex(new QMutex(QMutex::Recursive)),
    _buffers(),
    _idxRender(0),
    _idxDisplay(1),
    _newImage(false),
    _originalSize(),
    _targetSize(targetSize),
    _status(-1),
    _selfRestart(false),
    _deleteOnStop(false),
    _stopStatus(0),
    _firstImage(false),
    _originalTrack(INVALID_TRACK_ID)
{
    _buffers[0] = nullptr;
    _buffers[1] = nullptr;

#ifdef Q_OS_WIN
    _videoFile.replace("/", "\\\\");
#endif
    _vlcError = !VideoPlayer::initializeVLC();
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Player object initialized: " << this;
#endif
}

VideoPlayer::~VideoPlayer()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Destroying player object: " << this;
#endif

    _selfRestart = false;
    VideoPlayer::stopPlayer();
    VideoPlayer::cleanupBuffers();

    /*
    if(_vlcInstance)
    {
        libvlc_release(_vlcInstance);
        _vlcInstance = nullptr;
    }
    */

    delete _mutex;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Player object destroyed: " << this;
#endif

}

const QString& VideoPlayer::getFileName() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting file name: " << _videoFile;
#endif

    return _videoFile;
}

bool VideoPlayer::isPlayingVideo() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting playing video state: " << _playVideo;
#endif

    return _playVideo;
}

void VideoPlayer::setPlayingVideo(bool playVideo)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Setting playing video state: " << playVideo;
#endif

    _playVideo = playVideo;
}

bool VideoPlayer::isPlayingAudio() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting playing audio state: " << _playAudio;
#endif

    return _playAudio;
}

void VideoPlayer::setPlayingAudio(bool playAudio)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Setting playing audio state: " << playAudio;
#endif

    _playAudio = playAudio;

    // Layers: TODO
    /*
    if(!_vlcListPlayer)
        return;

    libvlc_media_player_t * player = libvlc_media_list_player_get_media_player(_vlcListPlayer);
    if(!player)
        return;

    if(_playAudio)
    {
        if((_originalTrack != INVALID_TRACK_ID) && (_originalTrack != -1))
            libvlc_audio_set_track(player, _originalTrack);
    }
    else
    {
        _originalTrack = libvlc_audio_get_track(player);
        if(_originalTrack != -1)
            libvlc_audio_set_track(player, -1);
    }
    */
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Playing audio state set";
#endif

}

bool VideoPlayer::isError() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting error state: " << _vlcError;
#endif

    return _vlcError;
}

QMutex* VideoPlayer::getMutex() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting mutex: " << _mutex;
#endif

    return _mutex;
}

QImage* VideoPlayer::getImage() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting image. Playing state: " << _status << ", display index: " << _idxDisplay << ", render index: " << _idxRender << ", display buffer: " << _buffers[_idxDisplay];
#endif

    return ((isPlaying()) && (_buffers[_idxDisplay])) ? _buffers[_idxDisplay]->getFrame() : nullptr;
}

QSize VideoPlayer::getOriginalSize() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting original size: " << _originalSize;
#endif

    return _originalSize;
}

bool VideoPlayer::isNewImage() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting new image state: " << _newImage;
#endif

    return _newImage;
}

void VideoPlayer::clearNewImage()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Clearing new image state";
#endif

    _newImage = false;
}

void* VideoPlayer::lockCallback(void **planes)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Lock callback called";
#endif

    _mutex->lock();

    if((planes) && (_buffers[_idxRender]) && (_buffers[_idxRender]->getNativeBuffer()))
    {
        *planes = _buffers[_idxRender]->getNativeBuffer();
    }

    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
        libvlc_clearerr();
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Lock completed";
#endif

    return nullptr;
}

void VideoPlayer::unlockCallback(void *picture, void *const *planes)
{
    Q_UNUSED(picture);
    Q_UNUSED(planes);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Unlock callback called. New Image: " << _newImage;
#endif

    if(!_mutex)
        return;

    if((!_buffers[0]) || (!_buffers[1]) || (_nativeWidth == 0) || (_nativeHeight == 0))
    {
        _mutex->unlock();
        return;
    }

    /*
    if(!_newImage)
    {
        // QImage::Format_RGB32 works fine as well and it's much faster
        _loadImage = QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_RGB32);
        _newImage = true;
    }
    */

    std::swap(_idxRender, _idxDisplay);
    _newImage = true;
    _mutex->unlock();

    if(!_firstImage)
    {
        _firstImage = true;
        emit screenShotAvailable();
    }

    emit frameAvailable();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Unlock completed";
#endif
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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Format callback called";
#endif

    if((!chroma) || (!width) || (!height) || (!pitches) || (!lines))
        return 0;

    if((_buffers[0]) || (_buffers[1]))
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

    _buffers[0] = new VideoPlayerImageBuffer(_nativeWidth, _nativeHeight);
    _buffers[1] = new VideoPlayerImageBuffer(_nativeWidth, _nativeHeight);
    _idxRender = 0;
    _idxDisplay = 1;

    /*
    _nativeBufferNotAligned = static_cast<uchar*>(malloc((_nativeWidth * _nativeHeight * 4) + 31));
    _nativeBuffer = reinterpret_cast<uchar*>((size_t(_nativeBufferNotAligned)+31) & static_cast<unsigned long long>(~31));
//    _loadImage = new QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_RGB32);
    _loadImage = new QImage(_nativeBuffer, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_ARGB32);
    */

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Format callback completed";
#endif

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

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Cleanup callback completed";
#endif
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

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Exit event callback completed";
#endif
}

void VideoPlayer::eventCallback(const struct libvlc_event_t *p_event)
{
    if(p_event)
    {
        switch(p_event->type)
        {
            case libvlc_MediaPlayerOpening:
                qDebug() << "[VideoPlayer] Video event received: OPENING = " << p_event->type;
                emit videoOpening();
                break;
            case libvlc_MediaPlayerBuffering:
                qDebug() << "[VideoPlayer] Video event received: BUFFERING = " << p_event->type;
                emit videoBuffering();
                break;
            case libvlc_MediaPlayerPlaying:
                qDebug() << "[VideoPlayer] Video event received: PLAYING = " << p_event->type;
                internalAudioCheck(p_event->type);
                emit videoPlaying();
                break;
            case libvlc_MediaPlayerPaused:
                qDebug() << "[VideoPlayer] Video event received: PAUSED = " << p_event->type;
                emit videoPaused();
                break;
            case libvlc_MediaPlayerStopped:
                qDebug() << "[VideoPlayer] Video event received: STOPPED = " << p_event->type;
                internalStopCheck(stopConfirmed);
                emit videoStopped();
                break;
            case libvlc_MediaListPlayerPlayed:
                qDebug() << "[VideoPlayer] Video event received: LIST PLAYED = " << p_event->type;
                break;
            case libvlc_MediaListPlayerStopped:
                qDebug() << "[VideoPlayer] Video event received: LIST STOPPED = " << p_event->type;
                internalStopCheck(stopConfirmed);
                emit videoStopped();
                break;
            default:
                qDebug() << "[VideoPlayer] UNEXPECTED Video event received:  " << p_event->type;
                break;
        };

        _status = p_event->type;
    }
}

void VideoPlayer::targetResized(const QSize& newSize)
{
    qDebug() << "[VideoPlayer] Target window resized: " << newSize;
    _targetSize = newSize;
    restartPlayer();
}

void VideoPlayer::stopThenDelete()
{
    if(isProcessing())
    {
        qDebug() << "[VideoPlayer] Stop Then Delete triggered, stop called...";
        _deleteOnStop = true;
        stopPlayer();
    }
    else
    {
        qDebug() << "[VideoPlayer] Stop Then Delete triggered, immediate delete possible.";
        delete this;
    }


#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] stopThenDelete completed";
#endif

}

bool VideoPlayer::restartPlayer()
{
    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayer] Restart Player called, stop called...";
        _selfRestart = true;
        return stopPlayer();
    }
    else
    {
        qDebug() << "[VideoPlayer] Restart Player called, but no player running - starting player!";
        return startPlayer();
    }
}

bool VideoPlayer::initializeVLC()
{
    qDebug() << "[VideoPlayer] Initializing VLC!";

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayer] ERROR: Playback file empty - not initializing VLC!";
        return false;
    }
/*
#ifdef VIDEO_DEBUG_MESSAGES
    const char *verbose_args = "-vvv";
    _vlcInstance = libvlc_new(1, &verbose_args);
#else
    _vlcInstance = libvlc_new(0, nullptr);
#endif

    if(!_vlcInstance)
        return false;

    libvlc_log_set(_vlcInstance, playerLogCallback, nullptr);

    libvlc_set_exit_handler(_vlcInstance, playerExitEventCallback, this);
*/

    DMH_VLC *vlcInstance = DMH_VLC::DMH_VLCInstance();
    if(!vlcInstance)
        return false;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Initializing VLC completed";
#endif

    return true;
}

bool VideoPlayer::startPlayer()
{
    if(!DMH_VLC::vlcInstance())
    {
        qDebug() << "[VideoPlayer] VLC not instantiated - not able to start player!";
        return false;
    }

    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayer] Player already running - not able to start player!";
        return false;
    }

    if(_videoFile.isEmpty())
    {
        qDebug() << "[VideoPlayer] Playback file empty - not able to start player!";
        return false;
    }

    qDebug() << "[VideoPlayer] Starting video player with " << _videoFile.toUtf8().constData();

    /*
    // Create a new Media List and add the media to it
    libvlc_media_list_t *vlcMediaList = libvlc_media_list_new();
    if (!vlcMediaList)
        return false;
        */

    // Create a new Media
#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)
    _vlcMedia = libvlc_media_new_path(_videoFile.toUtf8().constData());
#else
    _vlcMedia = libvlc_media_new_path(DMH_VLC::vlcInstance(), _videoFile.toUtf8().constData());
#endif
    //https://en.savefrom.net/18/
    //QString ytPath("https://r1---sn-w5nuxa-c33ey.googlevideo.com/videoplayback?expire=1597525099&ei=C_g3X8GwLLHU3LUP6dOm6A4&ip=14.207.129.148&id=o-AKlo5xUHtI-1uAnEPCm0wXnPupzmzuiOIXrUGtmT9WvJ&itag=22&source=youtube&requiressl=yes&mh=3O&mm=31%2C26&mn=sn-w5nuxa-c33ey%2Csn-npoe7ne6&ms=au%2Conr&mv=m&mvi=1&pl=23&initcwndbps=812500&vprv=1&mime=video%2Fmp4&ratebypass=yes&dur=167.090&lmt=1597239028450972&mt=1597503397&fvip=1&fexp=23883098&c=WEB&txp=6316222&sparams=expire%2Cei%2Cip%2Cid%2Citag%2Csource%2Crequiressl%2Cvprv%2Cmime%2Cratebypass%2Cdur%2Clmt&sig=AOq0QJ8wRQIgHwkUXh_YN2OS5o76bNa1APrbw3G4nMZgjVQQhMj7OUoCIQDesCxcrVOBSme7QNmar0mkG5U8fz_01LP3CAoXpmCwaQ%3D%3D&lsparams=mh%2Cmm%2Cmn%2Cms%2Cmv%2Cmvi%2Cpl%2Cinitcwndbps&lsig=AG3C_xAwRQIhAKjExXaqpMXxMk4sOFBoQBg6c7kfVKYnhFkv43RqJZ0JAiA10pSSMS4ozj73yfIXjEmcLEnqi5sqMEj9EvWTa3EVgg%3D%3D&contentlength=15083894&video_id=9bMTK0ml9ZI&title=%F0%9F%8E%B5+RPG+Boss+Battle+Music+-+Hydra");
    //libvlc_media_t *vlcMedia = libvlc_media_new_location(_vlcInstance, ytPath.toUtf8().constData());
    if (!_vlcMedia)
        return false;

    /*
    libvlc_media_list_add_media(vlcMediaList, vlcMedia);
    libvlc_media_release(vlcMedia);

    // Create a new libvlc player
    _vlcListPlayer = libvlc_media_list_player_new(_vlcInstance);
    if(!_vlcListPlayer)
        return false;
        */

    /*
    libvlc_media_player_t *player = libvlc_media_player_new(_vlcInstance);
    if(!player)
        return false;
    */

#if defined(Q_OS_WIN64) || defined(Q_OS_MAC)
    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), _vlcMedia);
#else
    _vlcPlayer = libvlc_media_player_new_from_media(_vlcMedia);
#endif
    if(!_vlcPlayer)
        return false;

    libvlc_media_release(_vlcMedia);
    _vlcMedia = nullptr;
    // TODO: Layers
    //libvlc_audio_set_volume(_vlcPlayer, 0);

    //libvlc_media_list_player_set_media_list(_vlcListPlayer, vlcMediaList);
    //libvlc_media_list_player_set_media_player(_vlcListPlayer, player);
    //libvlc_media_list_player_set_playback_mode(_vlcListPlayer, libvlc_playback_mode_loop);
    //libvlc_audio_set_volume(_vlcPlayer, 0);
    //libvlc_video_set_scale(_vlcPlayer, 0.25f);

    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerOpening, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerBuffering, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, playerEventCallback, static_cast<void*>(this));
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, playerEventCallback, static_cast<void*>(this));
    }

    //libvlc_event_manager_t* listEventManager = libvlc_media_list_player_event_manager(_vlcListPlayer);
    //if(eventManager)
    //{
    //    libvlc_event_attach(listEventManager, libvlc_MediaListPlayerPlayed, playerEventCallback, static_cast<void*>(this));
    //    libvlc_event_attach(listEventManager, libvlc_MediaListPlayerStopped, playerEventCallback, static_cast<void*>(this));
    //}

    unsigned x = 0;
    unsigned y = 0;
    int result = libvlc_video_get_size(_vlcPlayer, 0, &x, &y);
    qDebug() << "[VideoPlayer] Video size (result: " << result << "): " << x << " x " << y << ". File: " << _videoFile;

    libvlc_video_set_callbacks(_vlcPlayer,
                               playerLockCallback,
                               playerUnlockCallback,
                               playerDisplayCallback,
                               static_cast<void*>(this));
    libvlc_video_set_format_callbacks(_vlcPlayer,
                                      playerFormatCallback,
                                      playerCleanupCallback);

    // And start playback
    //libvlc_media_list_player_play(_vlcListPlayer);
    int playResult = libvlc_media_player_play(_vlcPlayer);
    result = libvlc_video_get_size(_vlcPlayer, 0, &x, &y);
    qDebug() << "[VideoPlayer] Video size (result: " << result << "): " << x << " x " << y << ". File: " << _videoFile;

    qDebug() << "[VideoPlayer] Player started: " << playResult;

    return true;
}

bool VideoPlayer::stopPlayer()
{
    if(_vlcPlayer)
    {
        qDebug() << "[VideoPlayer] Stop Player called";
        _stopStatus = 0;
        //libvlc_media_list_player_stop_async(_vlcListPlayer);
        libvlc_media_player_stop_async(_vlcPlayer);
        VideoPlayer::internalStopCheck(stopCallComplete);
    }

    return true;
}

void VideoPlayer::cleanupBuffers()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Cleaning up buffers";
#endif

    _newImage = false;
    _firstImage = false;
    _originalSize = QSize();

    QMutexLocker locker(_mutex);

    delete _buffers[0];
    _buffers[0] = nullptr;
    delete _buffers[1];
    _buffers[1] = nullptr;

    /*
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
    */

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Buffer cleanup completed";
#endif

}

void VideoPlayer::internalStopCheck(int status)
{
    _stopStatus |= status;

    qDebug() << "[VideoPlayer] Internal Stop Check called with status " << status << ", overall status: " << _stopStatus;

    if(_stopStatus != stopComplete)
        return;

    if(_vlcPlayer)
    {
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
        qDebug() << "[VideoPlayer] Internal Stop Check: vlc player destroyed.";
    }

    if(_vlcMedia)
    {
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
    }

    cleanupBuffers();

    if(_selfRestart)
    {
        _selfRestart = false;
        startPlayer();
        qDebug() << "[VideoPlayer] Internal Stop Check: player restarted.";
    }

    if(_deleteOnStop)
    {
        qDebug() << "[VideoPlayer] Internal Stop Check: video player being destroyed.";
//        delete this;
        return;
    }
}

void VideoPlayer::internalAudioCheck(int newStatus)
{
    if((_playAudio) ||
       (_originalTrack != INVALID_TRACK_ID) ||
       (newStatus != libvlc_MediaPlayerPlaying) ||
       (!_vlcPlayer))
        return;

    qDebug() << "[VideoPlayer] Internal Audio Check identified audio, shall be turned off";

    // TODO: Layers
    /*
    libvlc_media_player_t * player = libvlc_media_list_player_get_media_player(_vlcListPlayer);
    if(player)
    {
        _originalTrack = libvlc_audio_get_track(player);
        if(_originalTrack != -1)
            libvlc_audio_set_track(player, -1);
    }
    */

    qDebug() << "[VideoPlayer] Audio turning off completed";
}

bool VideoPlayer::isPlaying() const
{
    bool result = ((_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is playing status: " << result;
#endif

    return result;
}

bool VideoPlayer::isPaused() const
{
    bool result = (_status == libvlc_MediaPlayerPaused);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is paused status: " << result;
#endif

    return result;
}

bool VideoPlayer::isProcessing() const
{
    bool result = ((_status == libvlc_MediaPlayerOpening) ||
                   (_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying) ||
                   (_status == libvlc_MediaPlayerPaused));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is processing status: " << result;
#endif

    return result;
}

bool VideoPlayer::isStatusValid() const
{
    bool result = ((_status == libvlc_MediaPlayerOpening) ||
                   (_status == libvlc_MediaPlayerBuffering) ||
                   (_status == libvlc_MediaPlayerPlaying) ||
                   (_status == libvlc_MediaPlayerPaused) ||
                   (_status == libvlc_MediaPlayerStopped));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is status valid: " << result;
#endif

    return result;
}

VideoPlayer::VideoPlayerImageBuffer::VideoPlayerImageBuffer(unsigned int width, unsigned int height) :
    _nativeBufferNotAligned(nullptr),
    _nativeBuffer(nullptr),
    _imgFrame(nullptr)
{
    _nativeBufferNotAligned = static_cast<uchar*>(malloc((width * height * 4) + 31));
    _nativeBuffer = reinterpret_cast<uchar*>((size_t(_nativeBufferNotAligned)+31) & static_cast<unsigned long long>(~31));
    _imgFrame = new QImage(_nativeBuffer, static_cast<int>(width), static_cast<int>(height), QImage::Format_ARGB32);
}

VideoPlayer::VideoPlayerImageBuffer::~VideoPlayerImageBuffer()
{
    delete _imgFrame;

    if(_nativeBufferNotAligned)
    {
        unsigned char* tempChar = _nativeBufferNotAligned;
        free(tempChar);
    }
}

uchar* VideoPlayer::VideoPlayerImageBuffer::getNativeBuffer()
{
    return _nativeBuffer;
}

QImage* VideoPlayer::VideoPlayerImageBuffer::getFrame()
{
    return _imgFrame;
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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerLockCallback: " << opaque;
#endif

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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerUnlockCallback: " << opaque;
#endif

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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerDisplayCallback: " << opaque;
#endif

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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerFormatCallback: " << opaque;
#endif

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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerCleanupCallback: " << opaque;
#endif
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
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerExitEventCallback: " << opaque;
#endif
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

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] VLC Log: " << QString::vasprintf(fmt, args);
#endif

    return;
}

/**
 * Callback function notification
 * \param p_event the event triggering the callback
 */
void playerEventCallback(const struct libvlc_event_t *p_event, void *p_data)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerEventCallback: " << p_data;
#endif
    if(!p_data)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(p_data);
    player->eventCallback(p_event);
}

