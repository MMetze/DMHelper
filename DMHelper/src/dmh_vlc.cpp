#include "dmh_vlc.h"

#define VIDEO_DEBUG_MESSAGES

DMH_VLC* DMH_VLC::_instance = nullptr;

DMH_VLC::DMH_VLC() :
    _vlcInstance(nullptr)
{
#ifdef VIDEO_DEBUG_MESSAGES
//    const char *verbose_args = "-vvv";
//    _vlcInstance = libvlc_new(1, &verbose_args);

    /*
    // Special case version to create a new cache file
    const char *args[] = {
        "--reset-plugins-cache",
        "--plugins-cache",
        "--plugins-scan",
        ""
    };
    */

    // Normal run-time version
    const char *args[] = {
        "--no-reset-plugins-cache",
        "--plugins-cache",
        "--no-plugins-scan",
        ""
    };

    _vlcInstance = libvlc_new(sizeof(args) / sizeof(*args), args);

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

libvlc_instance_t* DMH_VLC::Instance()
{
    if(!_instance)
    {
        Initialize();
        if(!_instance)
            return nullptr;
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
        DMH_VLC* deleteInstance = _instance;
        _instance = nullptr;
        delete deleteInstance;
    }
}
