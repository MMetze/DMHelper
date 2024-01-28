#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QRecursiveMutex>
#include <QImage>
#include "dmh_vlc.h"

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
    virtual QRecursiveMutex* getMutex() const;
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

    void frameAvailable();
    void screenShotAvailable();

public slots:
    virtual void targetResized(const QSize& newSize);
    virtual void stopThenDelete();
    virtual bool restartPlayer();

protected slots:

    virtual void internalStopCheck(int status);

protected:

    virtual bool initializeVLC();
    virtual bool startPlayer();
    virtual bool stopPlayer();
    virtual void cleanupBuffers();

    virtual void handleVideoStopped();

    virtual bool isPlaying() const;
    virtual bool isPaused() const;
    virtual bool isProcessing() const;
    virtual bool isStatusValid() const;

    QString _videoFile;
    bool _playVideo;
    bool _playAudio;

    bool _vlcError;
    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;

    class VideoPlayerImageBuffer
    {
    public:
        VideoPlayerImageBuffer(unsigned int width, unsigned int height);
        ~VideoPlayerImageBuffer();

        uchar* getNativeBuffer();
        QImage* getFrame();

    private:
        uchar* _nativeBufferNotAligned;
        uchar* _nativeBuffer;
        QImage* _imgFrame;
    };

    unsigned int _nativeWidth;
    unsigned int _nativeHeight;
    QRecursiveMutex* _mutex;
    class VideoPlayerImageBuffer *_buffers[2];
    size_t _idxRender;
    size_t _idxDisplay;
    bool _newImage;
    QSize _originalSize;
    QSize _targetSize;
    int _status;
    bool _selfRestart;
    bool _deleteOnStop;
    int _stopStatus;
    int _frameCount;
    int _originalTrack;
};

#endif // VIDEOPLAYER_H
