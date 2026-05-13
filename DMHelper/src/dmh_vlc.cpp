#include "dmh_vlc.h"
#include "videoplayergl.h"
#include "videoplayerglvideo.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTimerEvent>
#include <QDebug>

//#define VIDEO_DEBUG_MESSAGES
//#define VIDEO_CREATE_CACHE

DMH_VLC* DMH_VLC::_instance = nullptr;

DMH_VLC::DMH_VLC(QObject *parent) :
    QObject(parent),
    _vlcInstance(nullptr),
    _currentVideo(nullptr)
{
#ifndef Q_OS_MAC
    bool needsReset = isCacheStale();

    if(needsReset)
    {
        qInfo() << "[DMH_VLC] VLC plugins cache is stale or missing, regenerating...";
        const char *args[] = {
            "--reset-plugins-cache",
            "--plugins-cache",
            "--plugins-scan",
            "--verbose=0",
            "--file-caching=100",
            "--clock-jitter=0"
        };
        _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);

        if(_vlcInstance)
            writeCacheSentinel();
    }
    else
    {
        const char *args[] = {
            "--no-reset-plugins-cache",
            "--plugins-cache",
            "--no-plugins-scan",
            "--verbose=0",
            "--file-caching=100",
            "--clock-jitter=0"
        };
        _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    }
#else
    _vlcInstance = libvlc_new(0, nullptr);
#endif
}

DMH_VLC::~DMH_VLC()
{
    if(_vlcInstance)
    {
        libvlc_release(_vlcInstance);
        _vlcInstance = nullptr;
    }
}

bool DMH_VLC::isCacheStale()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString sentinelPath = appDir + QString("/plugins/.vlc_cache_sentinel");

    QFile sentinel(sentinelPath);
    if(!sentinel.exists())
        return true;

    if(!sentinel.open(QIODevice::ReadOnly | QIODevice::Text))
        return true;

    QString storedDir = QString::fromUtf8(sentinel.readAll()).trimmed();
    sentinel.close();

    return (storedDir != appDir);
}

void DMH_VLC::writeCacheSentinel()
{
    QString appDir = QCoreApplication::applicationDirPath();
    QString sentinelPath = appDir + QString("/plugins/.vlc_cache_sentinel");

    QFile sentinel(sentinelPath);
    if(sentinel.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        sentinel.write(appDir.toUtf8());
        sentinel.close();
        qInfo() << "[DMH_VLC] VLC plugins cache regenerated for:" << appDir;
    }
}

DMH_VLC* DMH_VLC::DMH_VLCInstance()
{
    if(_instance)
        return _instance;

    Initialize();
    return _instance;
}

libvlc_instance_t* DMH_VLC::vlcInstance()
{
    if(!_instance)
    {
        Initialize();
        if(!_instance)
        {
            qDebug() << "[DMH_VLC] ERROR: unable to find or initialize a VLC instance for playback!";
            return nullptr;
        }
    }

    return _instance->_vlcInstance;
}

void DMH_VLC::Initialize()
{
    if(_instance)
        return;

    _instance = new DMH_VLC();
}

void DMH_VLC::Shutdown()
{
    if(_instance)
    {
        _instance->releaseVideo(_instance->_currentVideo);

        DMH_VLC* deleteInstance = _instance;
        _instance = nullptr;
        delete deleteInstance;
    }
}

VideoPlayerGLVideo* DMH_VLC::requestVideo(VideoPlayerGL* player)
{
    if((_currentVideo) || (!player))
        return nullptr;

    _currentVideo = new VideoPlayerGLVideo(player);
    qDebug() << "[DMH_VLC] New video created (" << reinterpret_cast<void *>(_currentVideo) << ") for player: " << reinterpret_cast<void *>(player);

    return _currentVideo;
}

bool DMH_VLC::releaseVideo(VideoPlayerGLVideo* video)
{
    if((!_currentVideo) || (video != _currentVideo))
        return false;

    delete _currentVideo;
    qDebug() << "[DMH_VLC] Video released: " << reinterpret_cast<void *>(_currentVideo);
    startTimer(1000);

    return true;
}

void DMH_VLC::timerEvent(QTimerEvent *event)
{
    if(!event)
        return;

    killTimer(event->timerId());
    _currentVideo = nullptr;
    qDebug() << "[DMH_VLC] Video now available";
    emit playerAvailable();
}

