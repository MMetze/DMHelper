#ifndef VIDEOPLAYERGLVIDEO_H
#define VIDEOPLAYERGLVIDEO_H

#include "dmh_vlc.h"
#include <QObject>
#include <QSemaphore>
#include <QMutex>
#include <QSize>

class VideoPlayerGL;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;

class VideoPlayerGLVideo : public QObject
{
    Q_OBJECT
public:
    VideoPlayerGLVideo(VideoPlayerGL* player);
    virtual ~VideoPlayerGLVideo();

    bool isNewFrameAvailable();
    QOpenGLFramebufferObject *getVideoFrame();
    QSize getVideoSize() const;

    static bool resizeRenderTextures(void* data, const libvlc_video_render_cfg_t *cfg,
                                     libvlc_video_output_cfg_t *render_cfg);
    static bool setup(void** data, const libvlc_video_setup_device_cfg_t *cfg,
                      libvlc_video_setup_device_info_t *out);
    static void cleanup(void* data);
    static void swap(void* data);
    static bool makeCurrent(void* data, bool current);
    static void* getProcAddress(void* data, const char* current);

protected slots:
    void configureContext(QOpenGLContext *renderContext);

protected:
    VideoPlayerGL *_player;
    QOpenGLContext *_context;
    QOffscreenSurface *_surface;
    QSemaphore _videoReady;

    //FBO data
    unsigned _width;
    unsigned _height;
    QMutex _textLock;
    QOpenGLFramebufferObject *_buffers[4];
    size_t _idxRender;
    size_t _idxSwapRender;
    size_t _idxSwapDisplay;
    size_t _idxDisplay;
    bool _updated;
    bool _initialized;
};

#endif // VIDEOPLAYERGLVIDEO_H
