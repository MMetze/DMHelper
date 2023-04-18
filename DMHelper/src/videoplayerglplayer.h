#ifndef VIDEOPLAYERGLPLAYER_H
#define VIDEOPLAYERGLPLAYER_H

#include "videoplayergl.h"
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

class VideoPlayerGLPlayer : public VideoPlayerGL
{
    Q_OBJECT
public:
    VideoPlayerGLPlayer(const QString& videoFile, QOpenGLContext* context, QSurfaceFormat format, bool playVideo = true, bool playAudio = true, QObject *parent = nullptr);
    virtual ~VideoPlayerGLPlayer();

    virtual const QString& getFileName() const;
    void paintGL();

    virtual bool isPlayingVideo() const;
    virtual void setPlayingVideo(bool playVideo);
    virtual bool isPlayingAudio() const;
    virtual void setPlayingAudio(bool playAudio);

    virtual bool isError() const;
    virtual QSize getOriginalSize() const;
    virtual void registerNewFrame() override;

    virtual QSurfaceFormat getFormat() const override;
    virtual QSize getSize() const override;

    QImage getLastScreenshot();

    bool vbObjectsExist();
    void createVBObjects();
    void cleanupVBObjects();

    static void playerEventCallback(const struct libvlc_event_t *p_event, void *p_data);

signals:
    void videoOpening();
    void videoPlaying();
    void videoBuffering();
    void videoPaused();
    void videoStopped();

    void frameAvailable();
    void vbObjectsCreated();

public slots:
    virtual void stopThenDelete();
    virtual bool restartPlayer();

    virtual void videoResized() override;

    void initializationComplete();

protected slots:
    void videoAvailable();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

    virtual bool initializeVLC() override;
    virtual bool startPlayer() override;
    virtual bool stopPlayer(bool restart) override;

    void cleanupPlayer();

    virtual void internalAudioCheck(int newStatus);

    virtual bool isPlaying() const;
    virtual bool isPaused() const;
    virtual bool isProcessing() const;
    virtual bool isStatusValid() const;

    QString _videoFile;
    QOpenGLContext* _context;
    QSurfaceFormat _format;
    QSize _videoSize;
    bool _playVideo;
    bool _playAudio;

    VideoPlayerGLVideo* _video;
    GLuint _fboTexture;

    bool _vlcError;
    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;
    int _status;
    bool _initialized;
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

#endif // VIDEOPLAYERGLPLAYER_H
