#ifndef DMH_VLC_H
#define DMH_VLC_H

#include <QtGlobal>

#ifdef Q_OS_WIN
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>


class DMH_VLC
{
public:
    explicit DMH_VLC();
    ~DMH_VLC();

    static libvlc_instance_t* Instance();

    static void Initialize();
    static void Shutdown();

private:
    static DMH_VLC* _instance;

    libvlc_instance_t* _vlcInstance;
};


#endif // DMH_VLC_H
