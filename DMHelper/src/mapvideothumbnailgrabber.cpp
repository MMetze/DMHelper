#include "mapvideothumbnailgrabber.h"

#ifdef DMH_MAP_VIDEO_THUMBNAILS

#include <QFile>
#include <QFileInfo>
#include <QDebug>

// Static VLC callback trampolines
static void* thumbLockCallback(void *opaque, void **planes)
{
    MapVideoThumbnailGrabber* grabber = static_cast<MapVideoThumbnailGrabber*>(opaque);
    if(grabber)
        return grabber->lockCallback(planes);
    return nullptr;
}

static void thumbUnlockCallback(void *opaque, void *picture, void *const *planes)
{
    MapVideoThumbnailGrabber* grabber = static_cast<MapVideoThumbnailGrabber*>(opaque);
    if(grabber)
        grabber->unlockCallback(picture, planes);
}

static void thumbDisplayCallback(void *opaque, void *picture)
{
    MapVideoThumbnailGrabber* grabber = static_cast<MapVideoThumbnailGrabber*>(opaque);
    if(grabber)
        grabber->displayCallback(picture);
}

static unsigned thumbFormatCallback(void **opaque, char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    MapVideoThumbnailGrabber* grabber = static_cast<MapVideoThumbnailGrabber*>(*opaque);
    if(grabber)
        return grabber->formatCallback(chroma, width, height, pitches, lines);
    return 0;
}

static void thumbCleanupCallback(void *opaque)
{
    MapVideoThumbnailGrabber* grabber = static_cast<MapVideoThumbnailGrabber*>(opaque);
    if(grabber)
        grabber->cleanupCallback();
}

static void thumbEventCallback(const struct libvlc_event_t *p_event, void *p_data)
{
    MapVideoThumbnailGrabber* grabber = static_cast<MapVideoThumbnailGrabber*>(p_data);
    if(grabber)
        grabber->eventCallback(p_event);
}

MapVideoThumbnailGrabber::MapVideoThumbnailGrabber(const QString& videoFile, const QString& cacheFile, int thumbnailSize, QObject *parent) :
    QObject(parent),
    _videoFile(videoFile),
    _cacheFile(cacheFile),
    _thumbnailSize(thumbnailSize),
    _vlcPlayer(nullptr),
    _vlcMedia(nullptr),
    _mutex(),
    _nativeBuffer(nullptr),
    _nativeBufferAligned(nullptr),
    _nativeWidth(0),
    _nativeHeight(0),
    _frameGrabbed(false),
    _stopping(false)
{
#ifdef Q_OS_WIN
    _videoFile.replace("/", "\\\\");
#endif
}

MapVideoThumbnailGrabber::~MapVideoThumbnailGrabber()
{
    cleanup();
}

void MapVideoThumbnailGrabber::start()
{
    if(!DMH_VLC::vlcInstance())
    {
        qDebug() << "[MapVideoThumbnailGrabber] VLC not initialized";
        emit thumbnailFailed(_videoFile);
        deleteLater();
        return;
    }

    if(_videoFile.isEmpty() || !QFile::exists(_videoFile))
    {
        qDebug() << "[MapVideoThumbnailGrabber] Video file not found: " << _videoFile;
        emit thumbnailFailed(_videoFile);
        deleteLater();
        return;
    }

    _vlcMedia = libvlc_media_new_path(_videoFile.toUtf8().constData());
    if(!_vlcMedia)
    {
        qDebug() << "[MapVideoThumbnailGrabber] Failed to create VLC media for: " << _videoFile;
        emit thumbnailFailed(_videoFile);
        deleteLater();
        return;
    }

    libvlc_media_add_option(_vlcMedia, ":avcodec-threads=0");
    libvlc_media_add_option(_vlcMedia, ":no-audio");

    _vlcPlayer = libvlc_media_player_new_from_media(DMH_VLC::vlcInstance(), _vlcMedia);
    if(!_vlcPlayer)
    {
        qDebug() << "[MapVideoThumbnailGrabber] Failed to create VLC player for: " << _videoFile;
        libvlc_media_release(_vlcMedia);
        _vlcMedia = nullptr;
        emit thumbnailFailed(_videoFile);
        deleteLater();
        return;
    }

    libvlc_media_release(_vlcMedia);
    _vlcMedia = nullptr;

    libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
    if(eventManager)
    {
        libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, thumbEventCallback, static_cast<void*>(this));
    }

    libvlc_video_set_callbacks(_vlcPlayer,
                               thumbLockCallback,
                               thumbUnlockCallback,
                               thumbDisplayCallback,
                               static_cast<void*>(this));

    libvlc_video_set_format_callbacks(_vlcPlayer,
                                      thumbFormatCallback,
                                      thumbCleanupCallback);

    libvlc_audio_set_volume(_vlcPlayer, 0);
    libvlc_media_player_play(_vlcPlayer);
}

void MapVideoThumbnailGrabber::handleStopped()
{
    cleanup();
    deleteLater();
}

void* MapVideoThumbnailGrabber::lockCallback(void **planes)
{
    _mutex.lock();
    if(_nativeBufferAligned)
        *planes = _nativeBufferAligned;
    return nullptr;
}

void MapVideoThumbnailGrabber::unlockCallback(void *picture, void *const *planes)
{
    Q_UNUSED(picture);
    Q_UNUSED(planes);
    _mutex.unlock();
}

void MapVideoThumbnailGrabber::displayCallback(void *picture)
{
    Q_UNUSED(picture);

    if(_frameGrabbed || _stopping)
        return;

    _frameGrabbed = true;

    // Grab the frame under lock
    QImage frame;
    {
        QMutexLocker locker(&_mutex);
        if(_nativeBufferAligned && _nativeWidth > 0 && _nativeHeight > 0)
        {
            QImage rawFrame(_nativeBufferAligned, static_cast<int>(_nativeWidth), static_cast<int>(_nativeHeight), QImage::Format_RGBA8888);
            frame = rawFrame.copy(); // deep copy before releasing the buffer
        }
    }

    if(!frame.isNull())
    {
        // Scale to thumbnail size
        QImage thumbnail = frame.scaled(_thumbnailSize, _thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        thumbnail.save(_cacheFile);
        qDebug() << "[MapVideoThumbnailGrabber] Thumbnail saved: " << _cacheFile;

        // Marshal signal back to GUI thread
        QMetaObject::invokeMethod(this, [this, thumbnail]() {
            emit thumbnailReady(_cacheFile, thumbnail);
        }, Qt::QueuedConnection);
    }
    else
    {
        QMetaObject::invokeMethod(this, [this]() {
            emit thumbnailFailed(_videoFile);
        }, Qt::QueuedConnection);
    }

    // Stop playback - marshal to GUI thread
    _stopping = true;
    QMetaObject::invokeMethod(this, [this]() {
        if(_vlcPlayer)
        {
            libvlc_media_player_stop_async(_vlcPlayer);
        }
    }, Qt::QueuedConnection);
}

unsigned MapVideoThumbnailGrabber::formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines)
{
    // Use RGBA format
    memcpy(chroma, "RV32", 4);

    _nativeWidth = *width;
    _nativeHeight = *height;

    *pitches = _nativeWidth * 4;
    *lines = _nativeHeight;

    QMutexLocker locker(&_mutex);
    delete[] _nativeBuffer;
    // Allocate with alignment
    size_t bufferSize = static_cast<size_t>(_nativeWidth) * _nativeHeight * 4 + 32;
    _nativeBuffer = new uchar[bufferSize];
    _nativeBufferAligned = reinterpret_cast<uchar*>((reinterpret_cast<uintptr_t>(_nativeBuffer) + 31) & ~static_cast<uintptr_t>(31));

    return 1;
}

void MapVideoThumbnailGrabber::cleanupCallback()
{
    QMutexLocker locker(&_mutex);
    delete[] _nativeBuffer;
    _nativeBuffer = nullptr;
    _nativeBufferAligned = nullptr;
    _nativeWidth = 0;
    _nativeHeight = 0;
}

void MapVideoThumbnailGrabber::eventCallback(const struct libvlc_event_t *p_event)
{
    if(!p_event)
        return;

    if(p_event->type == libvlc_MediaPlayerStopped)
    {
        QMetaObject::invokeMethod(this, &MapVideoThumbnailGrabber::handleStopped, Qt::QueuedConnection);
    }
}

void MapVideoThumbnailGrabber::cleanup()
{
    if(_vlcPlayer)
    {
        libvlc_event_manager_t* eventManager = libvlc_media_player_event_manager(_vlcPlayer);
        if(eventManager)
        {
            libvlc_event_detach(eventManager, libvlc_MediaPlayerStopped, thumbEventCallback, static_cast<void*>(this));
        }

        libvlc_video_set_callbacks(_vlcPlayer, nullptr, nullptr, nullptr, nullptr);
        libvlc_media_player_release(_vlcPlayer);
        _vlcPlayer = nullptr;
    }

    QMutexLocker locker(&_mutex);
    delete[] _nativeBuffer;
    _nativeBuffer = nullptr;
    _nativeBufferAligned = nullptr;
}

#endif // DMH_MAP_VIDEO_THUMBNAILS
