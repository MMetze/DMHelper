#include "dmh_vlc.h"
#include "videoplayergl.h"
#include "videoplayerglvideo.h"
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
    #ifdef VIDEO_CREATE_CACHE
        // Special case version to create a new cache file - only needed for a new version of VLC
        const char *args[] = {
            "--reset-plugins-cache",
            "--plugins-cache",
            "--plugins-scan",
            "-vvv",
            ""
        };

        _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);

    #else
        #ifdef VIDEO_DEBUG_MESSAGES
        //    const char *verbose_args = "-vvv";
        //    _vlcInstance = libvlc_new(1, &verbose_args);

            /*
            // Special case version to create a new cache file - only needed for a new version of VLC
            const char *args[] = {
                "--reset-plugins-cache",
                "--plugins-cache",
                "--plugins-scan",
        #ifdef QT_DEBUG
                "-vvv",
        #endif
                ""
            };
            */

            // Normal run-time version
            const char *args[] = {
                "--no-reset-plugins-cache",
                "--plugins-cache",
                "--no-plugins-scan",
        #ifdef QT_DEBUG
                "-vvv",
        #endif
                ""
            };

            _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);

        #else
//            _vlcInstance = libvlc_new(0, nullptr);
            // Normal run-time version
            const char *args[] = {
                "--no-reset-plugins-cache",
                "--plugins-cache",
                "--verbose=0",
                ""
            };

            _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
        #endif
    #endif
#else
    #ifdef VIDEO_DEBUG_MESSAGES
        // Normal run-time version
        const char *args[] = {
            "--no-reset-plugins-cache",
            "--plugins-cache",
            "--no-plugins-scan",
        #ifdef QT_DEBUG
                "-vvv",
        #endif
            ""
        };

        _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);
    #else
        _vlcInstance = libvlc_new(0, nullptr);
    #endif
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

