#ifndef VIDEOPLAYERGLVIDEO_H
#define VIDEOPLAYERGLVIDEO_H

#include "dmh_vlc.h"
#include <QSemaphore>
#include <QMutex>
#include <QSize>

class VideoPlayerGL;
class QOpenGLContext;
class QOpenGLFramebufferObject;
class QOffscreenSurface;

class VideoPlayerGLVideo
{
public:
    VideoPlayerGLVideo(VideoPlayerGL* player);
    ~VideoPlayerGLVideo();

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

private:
    VideoPlayerGL *_player;
    QOpenGLContext *_context;
    QOffscreenSurface *_surface;
    QSemaphore _videoReady;

    //FBO data
    unsigned _width = 0;
    unsigned _height = 0;
    QMutex _textLock;
    QOpenGLFramebufferObject *_buffers[3];
    size_t _idxRender = 0;
    size_t _idxSwap = 1;
    size_t _idxDisplay = 2;
    bool _updated = false;
};

#endif // VIDEOPLAYERGLVIDEO_H
