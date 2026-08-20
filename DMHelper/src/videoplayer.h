#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QMutex>
#include <QImage>
#include <QElapsedTimer>
#include <atomic>
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

    virtual int getVolume() const;
    virtual void setVolume(int volume);

    virtual void setLooping(bool looping);

    virtual bool isError() const;
    // lockMutex/unlockMutex are compatibility no-ops: getLockedImage() synchronizes internally
    virtual bool lockMutex();
    virtual void unlockMutex();
    virtual QImage* getLockedImage();
    virtual QSize getOriginalSize() const;
    virtual bool isNewImage() const;
    virtual void clearNewImage();

    // Perf stats: cumulative since object creation, incremented from VLC threads
    unsigned int getStatFramesDecoded() const;
    unsigned int getStatFramesDropped() const;
    qint64 getStatDecodeIntervalAccumNs() const;
    unsigned int getStatDecodeIntervalCount() const;
    // Returns the max decode interval since the last call and resets it (windowed max)
    qint64 takeStatDecodeIntervalMaxNs();

    virtual void* lockCallback(void **planes);
    virtual void unlockCallback(void *picture, void *const *planes);
    virtual void displayCallback(void *picture);
    virtual unsigned formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    virtual void cleanupCallback();
    virtual void exitEventCallback();
    virtual void eventCallback(libvlc_state_t state);

signals:
    // All signals are emitted from VLC's internal threads: receivers must use queued (or cross-thread auto) connections, never Qt::DirectConnection
    void videoOpening();
    void videoPlaying();
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
    int _volume;

    bool _vlcError;
    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;

    class VideoPlayerImageBuffer
    {
    public:
        VideoPlayerImageBuffer(unsigned int width, unsigned int height, unsigned int pitch, unsigned int lines);
        ~VideoPlayerImageBuffer();

        uchar* getNativeBuffer();
        QImage* getFrame();
        bool isValid() const;

    private:
        uchar* _nativeBufferNotAligned;
        uchar* _nativeBuffer;
        QImage* _imgFrame;
    };

    unsigned int _nativeWidth;
    unsigned int _nativeHeight;
    QMutex* _mutex;
    // Triple buffering: write is decoder-exclusive, display is consumer-exclusive, ready holds the latest complete frame
    class VideoPlayerImageBuffer *_buffers[3];
    VideoPlayerImageBuffer* _fallbackBuffer;
    size_t _idxWrite;
    size_t _idxReady;
    size_t _idxDisplay;
    std::atomic<bool> _newImage;
    QSize _originalSize;
    QSize _targetSize;
    std::atomic<int> _status;
    bool _looping;
    bool _selfRestart;
    bool _deleteOnStop;
    int _stopStatus;
    std::atomic<int> _frameCount;
    int _originalTrack;
    std::atomic<unsigned int> _statFramesDecoded;
    std::atomic<unsigned int> _statFramesDropped;
    QElapsedTimer _statDecodeIntervalTimer; // touched only on VLC's display thread
    std::atomic<qint64> _statDecodeIntervalAccumNs;
    std::atomic<unsigned int> _statDecodeIntervalCount;
    std::atomic<qint64> _statDecodeIntervalMaxNs;
};

#endif // VIDEOPLAYER_H
