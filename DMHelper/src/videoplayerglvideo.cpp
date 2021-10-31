#include "videoplayerglvideo.h"
#include "videoplayerglplayer.h"
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QDebug>

//#define VIDEO_DEBUG_MESSAGES

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
    _idxSwap(1),
    _idxDisplay(2),
    _updated(false)
{
    qDebug() << "[VideoPlayerGLVideo] Creating VideoPlayerGLVideo";

    _buffers[0] = nullptr;
    _buffers[1] = nullptr;
    _buffers[2] = nullptr;

    // Use default format for context
    _context = new QOpenGLContext(player);

    // Use offscreen surface to render the buffers
    _surface = new QOffscreenSurface(nullptr, player);

    // Player doesn't have an established OpenGL context right now, we'll get it later
    QObject::connect(player, &VideoPlayerGL::contextReady, [this](QOpenGLContext *renderContext)
    {
        if((!_player) || (!_surface) || (!_context))
            return;

        // Video view is now ready, we can start
        _surface->setFormat(_player->getFormat());
        _surface->create();

        _context->setFormat(_player->getFormat());
        if(renderContext)
            _context->setShareContext(renderContext);
        _context->create();

        _videoReady.release();
    });
}

VideoPlayerGLVideo::~VideoPlayerGLVideo()
{
    qDebug() << "[VideoPlayerGLVideo] Destroying VideoPlayerGLVideo";

    cleanup(this);
}

// Is there a new texture to be displayed
bool VideoPlayerGLVideo::isNewFrameAvailable()
{
    return _updated;
}

// Return the texture to be displayed
QOpenGLFramebufferObject *VideoPlayerGLVideo::getVideoFrame()
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLVideo] Video frame requested";
#endif

    QMutexLocker locker(&_textLock);

    if(_updated)
    {
        std::swap(_idxSwap, _idxDisplay);
        _updated = false;
    }
    return _buffers[_idxDisplay];
}

QSize VideoPlayerGLVideo::getVideoSize() const
{
    return QSize(static_cast<int>(_width), static_cast<int>(_height));
}

// This callback will create the surfaces and FBO used by VLC to perform its rendering
bool VideoPlayerGLVideo::resizeRenderTextures(void* data,
                                              const libvlc_video_render_cfg_t *cfg,
                                              libvlc_video_output_cfg_t *render_cfg)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);

    if((!that) || (!cfg) || (!render_cfg))
        return false;

    qDebug() << "[VideoPlayerGLVideo] Resizing render textures to: " << cfg->width << " x " << cfg->height;

    if((cfg->width != that->_width) || (cfg->height != that->_height))
        cleanup(data);

    if(!that->_buffers[0])
    {
        that->_buffers[0] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
        that->_buffers[1] = new QOpenGLFramebufferObject(cfg->width, cfg->height);
        that->_buffers[2] = new QOpenGLFramebufferObject(cfg->width, cfg->height);

        that->_width = cfg->width;
        that->_height = cfg->height;
    }

    that->_buffers[that->_idxRender]->bind();

    render_cfg->opengl_format = GL_RGBA;
    render_cfg->full_range = true;
    render_cfg->colorspace = libvlc_video_colorspace_BT709;
    render_cfg->primaries  = libvlc_video_primaries_BT709;
    render_cfg->transfer   = libvlc_video_transfer_func_SRGB;

    if(that->_player)
        that->_player->videoResized();

    return true;
}

// This callback is called during initialisation.
bool VideoPlayerGLVideo::setup(void** data,
                               const libvlc_video_setup_device_cfg_t *cfg,
                               libvlc_video_setup_device_info_t *out)
{
    Q_UNUSED(cfg);
    Q_UNUSED(out);

    qDebug() << "[VideoPlayerGLVideo] Setting up video";

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
    if(!that)
        return;

    qDebug() << "[VideoPlayerGLVideo] Cleaning up video";

    that->_videoReady.release();

    if((that->_width == 0) && (that->_height == 0))
        return;

    delete that->_buffers[0]; that->_buffers[0] = nullptr;
    delete that->_buffers[1]; that->_buffers[1] = nullptr;
    delete that->_buffers[2]; that->_buffers[2] = nullptr;
}

//This callback is called after VLC performs drawing calls
void VideoPlayerGLVideo::swap(void* data)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLVideo] Swapping video data";
#endif

    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    if((!that) || (!that->_player))
        return;

    QMutexLocker locker(&that->_textLock);
    std::swap(that->_idxSwap, that->_idxRender);
    that->_buffers[that->_idxRender]->bind();
    that->_updated = true;
    locker.unlock();

    that->_player->registerNewFrame();
}

// This callback is called to set the OpenGL context
bool VideoPlayerGLVideo::makeCurrent(void* data, bool current)
{
#ifdef VIDEO_DEBUG_MESSAGES
    qDebug() << "[VideoPlayerGLVideo] Making context current (current = " << current << ")";
#endif

    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    if(!that)
        return false;

    if(current)
        that->_context->makeCurrent(that->_surface);
    else
        that->_context->doneCurrent();

    return true;
}

// This callback is called by VLC to get OpenGL functions.
void* VideoPlayerGLVideo::getProcAddress(void* data, const char* current)
{
    VideoPlayerGLVideo* that = static_cast<VideoPlayerGLVideo*>(data);
    if(!that)
        return nullptr;

    qDebug() << "[VideoPlayerGLVideo] Querying proc address for: " << current;

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
