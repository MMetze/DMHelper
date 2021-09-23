#include "videoplayerglvideo.h"
#include "videoplayergl.h"
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>

VideoPlayerGLVideo::VideoPlayerGLVideo(VideoPlayerGL* player) :
    _player(player),
    _context(nullptr),
    _surface(nullptr),
    _videoReady(),
    _width(0),
    _height(0),
    _textLock(),
    _buffers(),
    _idxRender(0),
    _idxSwap1(1),
    _idxSwap2(2),
    _idxDisplay(3),
    _updated(false)
{
    _buffers[0] = nullptr;
    _buffers[1] = nullptr;
    _buffers[2] = nullptr;
    _buffers[3] = nullptr;

    // Use default format for context
    _context = new QOpenGLContext(player);

    // Use offscreen surface to render the buffers
    _surface = new QOffscreenSurface(nullptr, player);

    // Player doesn't have an established OpenGL context right now, we'll get it later
    QObject::connect(player, &VideoPlayerGL::contextReady, [this](QOpenGLContext *renderContext)
    {
        if((!_player) || (!_surface) || (!_context) || (!renderContext))
            return;

        // Video view is now ready, we can start
        _surface->setFormat(_player->getFormat());
        _surface->create();

        _context->setFormat(_player->getFormat());
        _context->setShareContext(renderContext);
        _context->create();

        _videoReady.release();
    });
}

VideoPlayerGLVideo::~VideoPlayerGLVideo()
{
    cleanup(this);
}

// Return the texture to be displayed
QOpenGLFramebufferObject *VideoPlayerGLVideo::getVideoFrame()
{
    QMutexLocker locker(&_textLock);

    if(_updated)
    {
        std::swap(_idxSwap1, _idxDisplay);
        _updated = false;
    }
    return _buffers[_idxDisplay];
}

// This callback will create the surfaces and FBO used by VLC to perform its rendering
bool VideoPlayerGLVideo::resizeRenderTextures(void* data,
                                              const libvlc_video_render_cfg_t *cfg,
                                              libvlc_video_output_cfg_t *render_cfg)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    if (cfg->width != that->_width || cfg->height != that->_height)
        cleanup(data);

    that->_buffers[0] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
    that->_buffers[1] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
    that->_buffers[2] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
    that->_buffers[3] = new QOpenGLFramebufferObject(cfg->width, cfg->height);

    that->_width = cfg->width;
    that->_height = cfg->height;

    that->_buffers[that->_idxRender]->bind();

    render_cfg->opengl_format = GL_RGBA;
    render_cfg->full_range = true;
    render_cfg->colorspace = libvlc_video_colorspace_BT709;
    render_cfg->primaries  = libvlc_video_primaries_BT709;
    render_cfg->transfer   = libvlc_video_transfer_func_SRGB;

    return true;
}

// This callback is called during initialisation.
bool VideoPlayerGLVideo::setup(void** data,
                               const libvlc_video_setup_device_cfg_t *cfg,
                               libvlc_video_setup_device_info_t *out)
{
    Q_UNUSED(cfg);
    Q_UNUSED(out);

    if (!QOpenGLContext::supportsThreadedOpenGL())
        return false;

    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(*data);

    // Wait for rendering view to be ready
    that->_videoReady.acquire();

    that->_width = 0;
    that->_height = 0;
    return true;
}


// This callback is called to release the texture and FBO created in resize
void VideoPlayerGLVideo::cleanup(void* data)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);

    that->_videoReady.release();

    if (that->_width == 0 && that->_height == 0)
        return;

    delete that->_buffers[0]; that->_buffers[0] = nullptr;
    delete that->_buffers[1]; that->_buffers[1] = nullptr;
    delete that->_buffers[2]; that->_buffers[2] = nullptr;
    delete that->_buffers[3]; that->_buffers[3] = nullptr;
}

//This callback is called after VLC performs drawing calls
void VideoPlayerGLVideo::swap(void* data)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    if((!that) || (!that->_player))
        return;

    QMutexLocker locker(&that->_textLock);
    std::swap(that->_idxSwap2, that->_idxSwap1);
    std::swap(that->_idxSwap2, that->_idxRender);

    that->_buffers[that->_idxRender]->bind();
    that->_updated = true;
    that->_player->registerNewFrame();
}

// This callback is called to set the OpenGL context
bool VideoPlayerGLVideo::makeCurrent(void* data, bool current)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    if (current)
        that->_context->makeCurrent(that->_surface);
    else
        that->_context->doneCurrent();
    return true;
}

// This callback is called by VLC to get OpenGL functions.
void* VideoPlayerGLVideo::getProcAddress(void* data, const char* current)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    /* Qt usual expects core symbols to be queryable, even though it's not
     * mentioned in the API. Cf QPlatformOpenGLContext::getProcAddress.
     * Thus, we don't need to wrap the function symbols here, but it might
     * fail to work (not crash though) on exotic platforms since Qt doesn't
     * commit to this behaviour. A way to handle this in a more stable way
     * would be to store the mContext->functions() table in a thread local
     * variable, and wrap every call to OpenGL in a function using this
     * thread local state to call the correct variant. */
    return reinterpret_cast<void*>(that->_context->getProcAddress(current));
}
