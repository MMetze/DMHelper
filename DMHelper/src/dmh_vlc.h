#ifndef DMH_VLC_H
#define DMH_VLC_H

#include <QtGlobal>
#include <QObject>

#ifdef Q_OS_WIN
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>

class VideoPlayerGL;
class VideoPlayerGLVideo;

class DMH_VLC: public QObject
{
    Q_OBJECT
public:
    explicit DMH_VLC(QObject *parent = nullptr);
    ~DMH_VLC();

    static DMH_VLC* DMH_VLCInstance();
    static libvlc_instance_t* vlcInstance();

    static void Initialize();
    static void Shutdown();

    VideoPlayerGLVideo* requestVideo(VideoPlayerGL* player);
    bool releaseVideo(VideoPlayerGLVideo* video);

signals:
    void playerAvailable();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

private:
    static DMH_VLC* _instance;

    libvlc_instance_t* _vlcInstance;
    VideoPlayerGLVideo* _currentVideo;
};


#endif // DMH_VLC_H
