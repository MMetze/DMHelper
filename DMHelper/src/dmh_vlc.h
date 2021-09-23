#ifndef DMH_VLC_H
#define DMH_VLC_H

#include <QtGlobal>

#ifdef Q_OS_WIN
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>

#endif // DMH_VLC_H
