#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QMutex>
#include <QImage>

#ifdef Q_OS_WIN
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#endif
#include <vlc/vlc.h>

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    VideoPlayer(const QString& videoFile, QSize targetSize, bool playVideo = true, bool playAudio = true, QObject *parent = nullptr);
    virtual ~VideoPlayer();

    virtual const QString& getFileName() const;

    virtual bool isPlayingVideo() const;
    virtual void setPlayingVideo(bool playVideo);
    virtual bool isPlayingAudio() const;
    virtual void setPlayingAudio(bool playAudio);

    virtual bool isError() const;
    virtual QMutex* getMutex() const;
    virtual QImage* getImage() const;
    virtual QSize getOriginalSize() const;
    virtual bool isNewImage() const;
    virtual void clearNewImage();

    virtual void* lockCallback(void **planes);
    virtual void unlockCallback(void *picture, void *const *planes);
    virtual void displayCallback(void *picture);
    virtual unsigned formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    virtual void cleanupCallback();
    virtual void exitEventCallback();
    virtual void eventCallback(const struct libvlc_event_t *p_event);

signals:
    void videoOpening();
    void videoPlaying();
    void videoBuffering();
    void videoPaused();
    void videoStopped();

    void screenShotAvailable();

public slots:
    virtual void targetResized(const QSize& newSize);
    virtual void stopThenDelete();
    virtual bool restartPlayer();

protected:

    virtual bool initializeVLC();
    virtual bool startPlayer();
    virtual bool stopPlayer();
    virtual void cleanupBuffers();

    virtual void internalStopCheck(int status);
    virtual void internalAudioCheck(int newStatus);

    virtual bool isPlaying() const;
    virtual bool isPaused() const;
    virtual bool isProcessing() const;
    virtual bool isStatusValid() const;

    QString _videoFile;
    bool _playVideo;
    bool _playAudio;

    bool _vlcError;
    libvlc_instance_t *_vlcInstance;
    libvlc_media_list_player_t *_vlcListPlayer;
    unsigned int _nativeWidth;
    unsigned int _nativeHeight;
    uchar* _nativeBufferNotAligned;
    uchar* _nativeBuffer;
    QMutex* _mutex;
    QImage* _loadImage;
    bool _newImage;
    QSize _originalSize;
    QSize _targetSize;
    int _status;
    bool _selfRestart;
    bool _deleteOnStop;
    int _stopStatus;
    bool _firstImage;
    int _originalTrack;
};

#endif // VIDEOPLAYER_H
