#ifndef MAPVIDEOTHUMBNAILGRABBER_H
#define MAPVIDEOTHUMBNAILGRABBER_H

#ifdef DMH_MAP_VIDEO_THUMBNAILS

#include <QObject>
#include <QMutex>
#include <QImage>
#include <QSize>
#include "dmh_vlc.h"

class MapVideoThumbnailGrabber : public QObject
{
    Q_OBJECT
public:
    explicit MapVideoThumbnailGrabber(const QString& videoFile, const QString& cacheFile, int thumbnailSize = 256, QObject *parent = nullptr);
    ~MapVideoThumbnailGrabber();

    void start();

signals:
    void thumbnailReady(const QString& cacheFile, const QImage& thumbnail);
    void thumbnailFailed(const QString& videoFile);

public slots:
    void handleStopped();

    // VLC callbacks (called from VLC thread - must not touch Qt GUI)
    void* lockCallback(void **planes);
    void unlockCallback(void *picture, void *const *planes);
    void displayCallback(void *picture);
    unsigned formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    void cleanupCallback();
    void eventCallback(const struct libvlc_event_t *p_event);

private:
    void cleanup();

    QString _videoFile;
    QString _cacheFile;
    int _thumbnailSize;

    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;

    QMutex _mutex;
    uchar* _nativeBuffer;
    uchar* _nativeBufferAligned;
    unsigned int _nativeWidth;
    unsigned int _nativeHeight;
    bool _frameGrabbed;
    bool _stopping;
};

#endif // DMH_MAP_VIDEO_THUMBNAILS

#endif // MAPVIDEOTHUMBNAILGRABBER_H
