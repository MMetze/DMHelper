#include "videoplayer.h"
#include <QFile>
#include <QDebug>

//#define VIDEO_DEBUG_MESSAGES

#ifdef VIDEO_DEBUG_MESSAGES
int COUNT_CALLBACKS = 0;
#endif

const int VIDEOPLAYER_STOP_CALL_STARTED = 0x01;
const int VIDEOPLAYER_STOP_CALL_COMPLETE = 0x02;
const int VIDEOPLAYER_STOP_CONFIRMED = 0x04;
const int VIDEOPLAYER_STOP_COMPLETE = VIDEOPLAYER_STOP_CALL_STARTED | VIDEOPLAYER_STOP_CALL_COMPLETE | VIDEOPLAYER_STOP_CONFIRMED;
const int INVALID_TRACK_ID = -99999;
const int VIDEOPLAYER_SCREENSHOT_FRAME = 3;
const unsigned int VIDEOPLAYER_BUFFER_ALIGNMENT = 32;
const int VIDEOPLAYER_DECODE_LOCK_TIMEOUT_MS = 2;

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
void playerStateChangedCallback(void *opaque, libvlc_state_t state);
void playerAudioPlayCallback(void *data, const void *samples, unsigned count, int64_t pts);

// cbs struct has static storage duration and carries no per-instance state; the opaque pointer supplies the instance
static const libvlc_media_player_cbs s_videoPlayerCbs = []() {
    libvlc_media_player_cbs cbs{};
    cbs.on_state_changed = &playerStateChangedCallback;
    return cbs;
}();


VideoPlayer::VideoPlayer(const QString& videoFile, QSize targetSize, bool playVideo, bool playAudio, QObject *parent) :
    QObject(parent),
    _videoFile(videoFile),
    _playVideo(playVideo),
    _playAudio(playAudio),
    _volume(100),
    _vlcError(false),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    _nativeWidth(0),
    _nativeHeight(0),
    _mutex(new QMutex()),
    _buffers(),
    _fallbackBuffer(nullptr),
    _idxWrite(0),
    _idxReady(1),
    _idxDisplay(2),
    _newImage(false),
    _originalSize(),
    _targetSize(targetSize),
    _status(-1),
    _looping(true),
    _selfRestart(false),
    _deleteOnStop(false),
    _stopStatus(0),
    _frameCount(0),
    _originalTrack(INVALID_TRACK_ID),
    _statFramesDecoded(0),
    _statFramesDropped(0),
    _statDecodeIntervalTimer(),
    _statDecodeIntervalAccumNs(0),
    _statDecodeIntervalCount(0),
    _statDecodeIntervalMaxNs(0)
{
    _buffers[0] = nullptr;
    _buffers[1] = nullptr;
    _buffers[2] = nullptr;

    connect(this, &VideoPlayer::videoStopped, this, &VideoPlayer::handleVideoStopped, Qt::QueuedConnection);

#ifdef Q_OS_WIN
    _videoFile.replace("/", "\\");
#endif
    _vlcError = !VideoPlayer::initializeVLC();
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Player object initialized: " << this << ", " << COUNT_CALLBACKS;
#endif
}

VideoPlayer::~VideoPlayer()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Destroying player object: " << this << ", " << COUNT_CALLBACKS;
#endif

    _selfRestart = false;
    _deleteOnStop = false;

    if(_vlcPlayer)
    {
        libvlc_media_player_stop_async(_vlcPlayer);

        // Release is the synchronization point: it blocks until VLC's internal threads have
        // finished, so no callbacks into this object can occur after it returns
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
    }

    if(_vlcMedia)
    {
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
    }

    VideoPlayer::cleanupBuffers();

    QMutex* deleteMutex = _mutex;
    _mutex = nullptr;
    delete deleteMutex;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Player object destroyed: " << this << ", " << COUNT_CALLBACKS;
#endif

}

const QString& VideoPlayer::getFileName() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting file name: " << _videoFile << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return _videoFile;
}

bool VideoPlayer::isPlayingVideo() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting playing video state: " << _playVideo << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return _playVideo;
}

void VideoPlayer::setPlayingVideo(bool playVideo)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Setting playing video state: " << playVideo << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    _playVideo = playVideo;
}

bool VideoPlayer::isPlayingAudio() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting playing audio state: " << _playAudio << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return _playAudio;
}

void VideoPlayer::setPlayingAudio(bool playAudio)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Setting playing audio state: " << playAudio << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    _playAudio = playAudio;
    if(_vlcPlayer)
        libvlc_audio_set_volume(_vlcPlayer, _playAudio ? _volume : 0);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Playing audio state set, " << this << ", " << COUNT_CALLBACKS;
#endif

}

int VideoPlayer::getVolume() const
{
    return _volume;
}

void VideoPlayer::setVolume(int volume)
{
    _volume = qBound(0, volume, 100);
    if(_vlcPlayer && _playAudio)
        libvlc_audio_set_volume(_vlcPlayer, _volume);
}

void VideoPlayer::setLooping(bool looping)
{
    _looping = looping;
}

bool VideoPlayer::isError() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting error state: " << _vlcError << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return _vlcError;
}

bool VideoPlayer::lockMutex()
{
    // Compatibility no-op: getLockedImage() synchronizes internally and the display buffer is consumer-exclusive
    return true;
}

void VideoPlayer::unlockMutex()
{
    // Compatibility no-op, see lockMutex()
}

QImage* VideoPlayer::getLockedImage()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Returning image. Playing state: " << _status << ", display index: " << _idxDisplay << ", ready index: " << _idxReady << ", write index: " << _idxWrite << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    if((!isPlaying()) || (!_mutex))
        return nullptr;

    QMutexLocker locker(_mutex);

    if(_newImage)
    {
        std::swap(_idxDisplay, _idxReady);
        _newImage = false;
    }

    return _buffers[_idxDisplay] ? _buffers[_idxDisplay]->getFrame() : nullptr;
}

QSize VideoPlayer::getOriginalSize() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting original size: " << _originalSize << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return _originalSize;
}

bool VideoPlayer::isNewImage() const
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting new image state: " << _newImage << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return _newImage;
}

void VideoPlayer::clearNewImage()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Clearing new image state" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    _newImage = false;
}

unsigned int VideoPlayer::getStatFramesDecoded() const
{
    return _statFramesDecoded;
}

unsigned int VideoPlayer::getStatFramesDropped() const
{
    return _statFramesDropped;
}

qint64 VideoPlayer::getStatDecodeIntervalAccumNs() const
{
    return _statDecodeIntervalAccumNs;
}

unsigned int VideoPlayer::getStatDecodeIntervalCount() const
{
    return _statDecodeIntervalCount;
}

qint64 VideoPlayer::takeStatDecodeIntervalMaxNs()
{
    return _statDecodeIntervalMaxNs.exchange(0);
}

void* VideoPlayer::lockCallback(void **planes)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Lock callback called" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    if(!planes)
        return nullptr;

    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
#endif
        libvlc_clearerr();
    }

    // This callback cannot fail: *planes must always point at writable memory or VLC will corrupt memory.
    // The lock is only needed to synchronize the buffer pointer read with (re)allocation; the write buffer
    // itself is decoder-exclusive.
    if((_mutex) && (_mutex->tryLock(VIDEOPLAYER_DECODE_LOCK_TIMEOUT_MS)))
    {
        VideoPlayerImageBuffer* writeBuffer = _buffers[_idxWrite];
        _mutex->unlock();
        if((writeBuffer) && (writeBuffer->getNativeBuffer()))
        {
            *planes = writeBuffer->getNativeBuffer();
            return writeBuffer;
        }
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Lock callback failing forward, frame will be dropped" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    ++_statFramesDropped;

    // Fail forward: decode into the fallback buffer; the null token tells unlockCallback to drop the frame
    *planes = _fallbackBuffer ? _fallbackBuffer->getNativeBuffer() : nullptr;
    return nullptr;
}

void VideoPlayer::unlockCallback(void *picture, void *const *planes)
{
    Q_UNUSED(planes);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Unlock callback called. New Image: " << _newImage << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    // A null token means lockCallback failed forward into the fallback buffer - drop the frame
    if((!picture) || (!_mutex))
        return;

    // Drop the frame rather than stall the decode thread on contention
    if(!_mutex->tryLock(VIDEOPLAYER_DECODE_LOCK_TIMEOUT_MS))
    {
        ++_statFramesDropped;
        return;
    }

    if(_buffers[_idxWrite] == static_cast<VideoPlayerImageBuffer*>(picture))
    {
        std::swap(_idxWrite, _idxReady);
        _newImage = true;
    }
    _mutex->unlock();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Unlock completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
}

void VideoPlayer::displayCallback(void *picture)
{
    Q_UNUSED(picture);

    ++_statFramesDecoded;

    // Delivery jitter on VLC's own clock: distinguishes uneven decode from late GUI-thread presentation
    if(_statDecodeIntervalTimer.isValid())
    {
        const qint64 intervalNs = _statDecodeIntervalTimer.nsecsElapsed();
        _statDecodeIntervalAccumNs += intervalNs;
        ++_statDecodeIntervalCount;
        qint64 prevMax = _statDecodeIntervalMaxNs.load();
        while((intervalNs > prevMax) && (!_statDecodeIntervalMaxNs.compare_exchange_weak(prevMax, intervalNs))) {}
    }
    _statDecodeIntervalTimer.start();

    if(++_frameCount == VIDEOPLAYER_SCREENSHOT_FRAME)
        emit screenShotAvailable();

    emit frameAvailable();

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Display callback called" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
}

unsigned VideoPlayer::formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Format callback called" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    if((!chroma) || (!width) || (!height) || (!pitches) || (!lines))
        return 0;

    if(!_mutex)
        return 0;

    QMutexLocker locker(_mutex);

    if((_buffers[0]) || (_buffers[1]) || (_buffers[2]))
        return 0;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Format Callback with chroma: " << QString(chroma) << ", width: " << *width << ", height: " << *height << ", pitches: " << *pitches << ", lines: " << *lines << ", " << this;
#endif

    // RGBA matches QImage::Format_RGBA8888, so the GL upload path needs no CPU swizzle;
    // VLC performs the conversion on its worker thread at unchanged cost
    memcpy(chroma, "RGBA", sizeof("RGBA") - 1);

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

    // Pad pitch and line count to the alignment: VLC's optimized converters may write full vectors per scanline
    const unsigned int bufferPitch = ((_nativeWidth * 4) + VIDEOPLAYER_BUFFER_ALIGNMENT - 1) & ~(VIDEOPLAYER_BUFFER_ALIGNMENT - 1);
    const unsigned int bufferLines = (_nativeHeight + VIDEOPLAYER_BUFFER_ALIGNMENT - 1) & ~(VIDEOPLAYER_BUFFER_ALIGNMENT - 1);
    *pitches = bufferPitch;
    *lines = bufferLines;

    delete _fallbackBuffer;
    _fallbackBuffer = new VideoPlayerImageBuffer(_nativeWidth, _nativeHeight, bufferPitch, bufferLines);
    _buffers[0] = new VideoPlayerImageBuffer(_nativeWidth, _nativeHeight, bufferPitch, bufferLines);
    _buffers[1] = new VideoPlayerImageBuffer(_nativeWidth, _nativeHeight, bufferPitch, bufferLines);
    _buffers[2] = new VideoPlayerImageBuffer(_nativeWidth, _nativeHeight, bufferPitch, bufferLines);
    if((!_fallbackBuffer->isValid()) || (!_buffers[0]->isValid()) || (!_buffers[1]->isValid()) || (!_buffers[2]->isValid()))
    {
        delete _fallbackBuffer;
        _fallbackBuffer = nullptr;
        delete _buffers[0];
        _buffers[0] = nullptr;
        delete _buffers[1];
        _buffers[1] = nullptr;
        delete _buffers[2];
        _buffers[2] = nullptr;
        return 0;
    }
    _idxWrite = 0;
    _idxReady = 1;
    _idxDisplay = 2;
    _newImage = false;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Format callback completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return 1;
}

void VideoPlayer::cleanupCallback()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Cleanup Callback" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
#endif
        libvlc_clearerr();
    }

    // Marshal to the object's thread so buffer deletion can never race a consumer copying the display frame
    QMetaObject::invokeMethod(this, &VideoPlayer::cleanupBuffers, Qt::QueuedConnection);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Cleanup callback completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
}

void VideoPlayer::exitEventCallback()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Exit Event Callback" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
    const char * errmsg = libvlc_errmsg();
    if(errmsg)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] VLC ERROR: " << errmsg;
#endif
        libvlc_clearerr();
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Exit event callback completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
}

void VideoPlayer::eventCallback(libvlc_state_t state)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Event callback called. state: " << state << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    switch(state)
    {
        case libvlc_Opening:
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayer] Video event received: OPENING = " << state << ", " << COUNT_CALLBACKS;
#endif
            emit videoOpening();
            break;
        case libvlc_Playing:
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayer] Video event received: PLAYING = " << state << ", " << COUNT_CALLBACKS;
#endif
            emit videoPlaying();
            break;
        case libvlc_Paused:
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayer] Video event received: PAUSED = " << state << ", " << COUNT_CALLBACKS;
#endif
            emit videoPaused();
            break;
        case libvlc_Stopped:
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayer] Video event received: STOPPED = " << state << ", " << COUNT_CALLBACKS;
#endif
            emit videoStopped();
            break;
        default:
#ifdef VIDEO_DEBUG_MESSAGES
            qDebug() << "[VideoPlayer] UNEXPECTED Video event received:  " << state << ", " << COUNT_CALLBACKS;
#endif
            break;
    };

    _status = state;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Event callback completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
}

void VideoPlayer::targetResized(const QSize& newSize)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Target window resized: " << newSize << ", " << this << ", " << COUNT_CALLBACKS;
#endif
    _targetSize = newSize;
    restartPlayer();
}

void VideoPlayer::stopThenDelete()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] stopThenDelete called, " << this << ", " << COUNT_CALLBACKS;
#endif

    // Never delete synchronously: VLC worker threads may still be inside the
    // open / decode callbacks for this object (especially right after
    // startPlayer(), before any status event has arrived). Deleting here would
    // free the object out from under those threads (use-after-free inside
    // libVLC). If a player exists, stop it and defer destruction until the
    // Stopped event is confirmed in internalStopCheck(); otherwise it is safe
    // to schedule deletion on the event loop.
    if(_vlcPlayer)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Stop Then Delete triggered, stop called, " << this << ", " << COUNT_CALLBACKS;
#endif
        _deleteOnStop = true;
        stopPlayer();
    }
    else
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Stop Then Delete triggered, no player running - deferred delete, " << this << ", " << COUNT_CALLBACKS;
#endif
        deleteLater();
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] stopThenDelete completed, " << COUNT_CALLBACKS;
#endif

}

bool VideoPlayer::restartPlayer()
{
    if(_vlcPlayer)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Restart Player called, stop called..." << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        _selfRestart = true;
        return stopPlayer();
    }
    else
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Restart Player called, but no player running - starting player!" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        return startPlayer();
    }
}

void VideoPlayer::internalStopCheck(int status)
{
    _stopStatus |= status;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Internal Stop Check called with status " << status << ", overall status: " << _stopStatus << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    // Check if the video just ended and should be restarted
    if(_stopStatus == VIDEOPLAYER_STOP_CONFIRMED)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Internal Stop Check: Video ended, restarting playback" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        _stopStatus = 0;
        if(_vlcPlayer)
        {
            if(_playAudio)
            {
                int currentVolume = libvlc_audio_get_volume(_vlcPlayer);
                if(currentVolume >= 0)
                    _volume = currentVolume;
            }
            libvlc_media_player_release(_vlcPlayer);
            _vlcPlayer = nullptr;
        }
        if(_looping)
            startPlayer();
        return;
    }

    // Check if the video is not yet fully stopped
    if(_stopStatus != VIDEOPLAYER_STOP_COMPLETE)
        return;

    if(_vlcPlayer)
    {
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Internal Stop Check: VLC player destroyed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
    }

    cleanupBuffers();

    if(_deleteOnStop)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Internal Stop Check: video player being destroyed." << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        _deleteOnStop = false;
        deleteLater();
        return;
    }

    if(_selfRestart)
    {
        _selfRestart = false;
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Internal Stop Check: player restarting" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        startPlayer();
    }
}

bool VideoPlayer::initializeVLC()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Initializing VLC!" << ", " << this;
#endif

    if(_videoFile.isEmpty())
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] ERROR: Playback file empty - not initializing VLC!" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        return false;
    }

    DMH_VLC *vlcInstance = DMH_VLC::DMH_VLCInstance();
    if(!vlcInstance)
        return false;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Initializing VLC completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return true;
}

bool VideoPlayer::startPlayer()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Starting player " << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    if(!DMH_VLC::vlcInstance())
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] ERROR: VLC not instantiated - not able to start player!" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        return false;
    }

    if(_vlcPlayer)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Player already running - not able to start player!" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        return false;
    }

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Starting video player with " << _videoFile.toUtf8().constData() << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    if(_videoFile.isEmpty())
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Playback file empty - not able to start player!" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        return false;
    }

    if(!QFile::exists(_videoFile))
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Playback file does not exist - not able to start player!" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        return false;
    }

    // Create a new Media
    _vlcMedia = libvlc_media_new_path(_videoFile.toUtf8().constData());
    if (!_vlcMedia)
        return false;

    libvlc_media_add_option(_vlcMedia, ":avcodec-threads=0");

    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), _vlcMedia, &s_videoPlayerCbs, static_cast<void*>(this));
    if(!_vlcPlayer)
    {
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
        return false;
    }

    libvlc_media_release(_vlcMedia);
    _vlcMedia = nullptr;

    libvlc_video_set_callbacks(_vlcPlayer,
                               playerLockCallback,
                               playerUnlockCallback,
                               playerDisplayCallback,
                               static_cast<void*>(this));

    libvlc_video_set_format_callbacks(_vlcPlayer,
                                      playerFormatCallback,
                                      playerCleanupCallback);

    // And start playback
#ifdef VIDEO_DEBUG_MESSAGES
    int playResult = libvlc_media_player_play(_vlcPlayer);
#else
    libvlc_media_player_play(_vlcPlayer);
#endif
    libvlc_audio_set_volume(_vlcPlayer, _playAudio ? _volume : 0);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Player started: " << playResult << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return true;
}

bool VideoPlayer::stopPlayer()
{
    if(_vlcPlayer)
    {
#ifdef VIDEO_DEBUG_MESSAGES
        qDebug() << "[VideoPlayer] Stop Player called" << ", " << this << ", " << COUNT_CALLBACKS;
#endif
        _stopStatus = VIDEOPLAYER_STOP_CALL_STARTED;
        libvlc_media_player_stop_async(_vlcPlayer);
        VideoPlayer::internalStopCheck(VIDEOPLAYER_STOP_CALL_COMPLETE);
    }

    return true;
}

void VideoPlayer::handleVideoStopped()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Handling video stopped, " << this << ", " << COUNT_CALLBACKS;
#endif

    internalStopCheck(VIDEOPLAYER_STOP_CONFIRMED);
}

void VideoPlayer::cleanupBuffers()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Cleaning up buffers" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    if(!_mutex)
        return;

    QMutexLocker locker(_mutex);

    _newImage = false;
    _frameCount = 0;
    _originalSize = QSize();

    delete _buffers[0];
    _buffers[0] = nullptr;
    delete _buffers[1];
    _buffers[1] = nullptr;
    delete _buffers[2];
    _buffers[2] = nullptr;
    delete _fallbackBuffer;
    _fallbackBuffer = nullptr;

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Buffer cleanup completed" << ", " << this << ", " << COUNT_CALLBACKS;
#endif

}

bool VideoPlayer::isPlaying() const
{
    bool result = (_status == libvlc_Playing);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is playing status: " << result << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return result;
}

bool VideoPlayer::isPaused() const
{
    bool result = (_status == libvlc_Paused);

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is paused status: " << result << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return result;
}

bool VideoPlayer::isProcessing() const
{
    bool result = ((_status == libvlc_Opening) ||
                   (_status == libvlc_Playing) ||
                   (_status == libvlc_Paused));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is processing status: " << result << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return result;
}

bool VideoPlayer::isStatusValid() const
{
    bool result = ((_status == libvlc_Opening) ||
                   (_status == libvlc_Playing) ||
                   (_status == libvlc_Paused) ||
                   (_status == libvlc_Stopped));

#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] Getting is status valid: " << result << ", " << this << ", " << COUNT_CALLBACKS;
#endif

    return result;
}

VideoPlayer::VideoPlayerImageBuffer::VideoPlayerImageBuffer(unsigned int width, unsigned int height, unsigned int pitch, unsigned int lines) :
    _nativeBufferNotAligned(nullptr),
    _nativeBuffer(nullptr),
    _imgFrame(nullptr)
{
    const size_t bufferSize = (static_cast<size_t>(pitch) * static_cast<size_t>(lines)) + VIDEOPLAYER_BUFFER_ALIGNMENT - 1;
    _nativeBufferNotAligned = static_cast<uchar*>(malloc(bufferSize));
    if(!_nativeBufferNotAligned)
        return;

    _nativeBuffer = reinterpret_cast<uchar*>((reinterpret_cast<size_t>(_nativeBufferNotAligned) + VIDEOPLAYER_BUFFER_ALIGNMENT - 1) & ~(static_cast<size_t>(VIDEOPLAYER_BUFFER_ALIGNMENT) - 1));
    _imgFrame = new QImage(_nativeBuffer, static_cast<int>(width), static_cast<int>(height), static_cast<int>(pitch), QImage::Format_RGBA8888);
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

bool VideoPlayer::VideoPlayerImageBuffer::isValid() const
{
    return (_nativeBuffer != nullptr) && (_imgFrame != nullptr);
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
    qDebug() << "[VideoPlayer] playerLockCallback: " << opaque << ", " << COUNT_CALLBACKS++;
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
    qDebug() << "[VideoPlayer] playerUnlockCallback: " << opaque << ", " << COUNT_CALLBACKS++;
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
    qDebug() << "[VideoPlayer] playerDisplayCallback: " << opaque << ", " << COUNT_CALLBACKS++;
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
    qDebug() << "[VideoPlayer] playerFormatCallback: " << opaque << ", " << COUNT_CALLBACKS++;
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
    qDebug() << "[VideoPlayer] playerCleanupCallback: " << opaque << ", " << COUNT_CALLBACKS++;
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
    qDebug() << "[VideoPlayer] playerExitEventCallback: " << opaque << ", " << COUNT_CALLBACKS++;
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
    qDebug() << "[VideoPlayer] VLC Log: " << QString::vasprintf(fmt, args) << ", " << COUNT_CALLBACKS++;
#endif

    return;
}

/**
 * Callback function notification
 * \param opaque the VideoPlayer instance passed at player creation
 * \param state the new player state
 */
void playerStateChangedCallback(void *opaque, libvlc_state_t state)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayer] playerStateChangedCallback: " << opaque << ", " << COUNT_CALLBACKS++;
#endif
    if(!opaque)
        return;

    VideoPlayer* player = static_cast<VideoPlayer*>(opaque);
    player->eventCallback(state);
}

void playerAudioPlayCallback(void *data, const void *samples, unsigned count, int64_t pts)
{
    Q_UNUSED(data);
    Q_UNUSED(samples);
    Q_UNUSED(count);
    Q_UNUSED(pts);
}
