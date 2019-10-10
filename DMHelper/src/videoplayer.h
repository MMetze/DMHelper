#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QObject>
#include <QMutex>
#include <QImage>


#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#include <vlc/vlc.h>

class VideoPlayer : public QObject
{
    Q_OBJECT
public:
    VideoPlayer(QSize targetSize, QObject *parent = nullptr);
    virtual ~VideoPlayer();

    virtual bool isError() const;
    virtual QMutex* getMutex() const;
    virtual QImage* getImage() const;
    virtual bool isNewImage() const;
    virtual void clearNewImage();

    virtual void* lockCallback(void **planes);
    virtual void unlockCallback(void *picture, void *const *planes);
    virtual void displayCallback(void *picture);
    virtual unsigned formatCallback(char *chroma, unsigned *width, unsigned *height, unsigned *pitches, unsigned *lines);
    virtual void cleanupCallback();
    virtual void exitEventCallback();
    virtual void eventCallback(const struct libvlc_event_t *p_event);

public slots:
    virtual void targetResized(const QSize& newSize);

protected:

    virtual bool initializeVLC();
    virtual void cleanupBuffers();

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
    QSize _targetSize;
    int _status;
};

#endif // VIDEOPLAYER_H
