#ifndef VIDEOPLAYERGL_H
#define VIDEOPLAYERGL_H

#include <QObject>
#include <QMutex>
#include <QImage>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObject>
#include <QOffscreenSurface>
#include <QSemaphore>
#include <QMatrix4x4>
#include "dmh_vlc.h"

class VideoPlayerGLVideo;

class VideoPlayerGL : public QObject
{
    Q_OBJECT
public:
    VideoPlayerGL(const QString& videoFile, QOpenGLContext* context, QSurfaceFormat format, QSize targetSize, bool playVideo = true, bool playAudio = true, QObject *parent = nullptr);
    virtual ~VideoPlayerGL();

    virtual const QString& getFileName() const;
//    QOpenGLFramebufferObject* getVideoFrame();
    void paintGL();

    virtual bool isPlayingVideo() const;
    virtual void setPlayingVideo(bool playVideo);
    virtual bool isPlayingAudio() const;
    virtual void setPlayingAudio(bool playAudio);

    virtual bool isError() const;
    //virtual QMutex* getMutex();
    virtual QSize getOriginalSize() const;
    //virtual bool isNewImage() const;
    //virtual void clearNewImage();
    virtual void registerNewFrame();

    const QSurfaceFormat &getFormat() const;

    // libvlc callback static functions
    /*
    static bool resizeRenderTextures(void* data, const libvlc_video_render_cfg_t *cfg, libvlc_video_output_cfg_t *render_cfg);
    static bool setup(void** data, const libvlc_video_setup_device_cfg_t *cfg, libvlc_video_setup_device_info_t *out);
    static void cleanup(void* data);
    static void swap(void* data);
    static bool makeCurrent(void* data, bool current);
    static void* getProcAddress(void* data, const char* current);
    static void playerLogCallback(void *data, int level, const libvlc_log_t *ctx, const char *fmt, va_list args);
    */

    /*
    virtual void* lockCallback(void **planes);
    virtual void unlockCallback(void *picture, void *const *planes);
    virtual void displayCallback(void *picture);
    virtual unsigned formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    virtual void cleanupCallback();
    virtual void exitEventCallback();
    virtual void eventCallback(const struct libvlc_event_t *p_event);
    */

signals:
    void videoOpening();
    void videoPlaying();
    void videoBuffering();
    void videoPaused();
    void videoStopped();

    void screenShotAvailable();
    void frameAvailable();

    void contextReady(QOpenGLContext *renderContext);

public slots:
    virtual void targetResized(const QSize& newSize);
    virtual void stopThenDelete();
    virtual bool restartPlayer();

    void initializationComplete();
    void videoResized();

protected:

    virtual bool initializeVLC();
    virtual bool startPlayer();
    virtual bool stopPlayer();
    //virtual void cleanupBuffers();
    void createGLObjects();
    void cleanupGLObjects();
    void createVBObjects();
    void cleanupVBObjects();

//    virtual void internalStopCheck(int status);
    virtual void internalAudioCheck(int newStatus);

    virtual bool isPlaying() const;
    virtual bool isPaused() const;
    virtual bool isProcessing() const;
    virtual bool isStatusValid() const;

    QString _videoFile;
    QOpenGLContext* _context;
    QSurfaceFormat _format;
    bool _playVideo;
    bool _playAudio;

    VideoPlayerGLVideo* _video;
    GLuint _tempTexture;

    bool _vlcError;
    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;
    //QMutex _mutex;
    //bool _newImage;
    //QSize _originalSize;
    QSize _targetSize;
    int _status;
    bool _selfRestart;
    bool _deleteOnStop;
    int _stopStatus;
    bool _firstImage;
    int _originalTrack;

    QMatrix4x4 _modelMatrix;
    unsigned int _VAO;
    unsigned int _VBO;
    unsigned int _EBO;

};

#endif // VIDEOPLAYERGL_H
