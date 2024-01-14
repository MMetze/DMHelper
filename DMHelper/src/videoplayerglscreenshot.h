#ifndef VIDEOPLAYERGLSCREENSHOT_H
#define VIDEOPLAYERGLSCREENSHOT_H

#include "videoplayergl.h"
#include "dmh_vlc.h"

class VideoPlayerGLVideo;

class VideoPlayerGLScreenshot : public VideoPlayerGL
{
    Q_OBJECT
public:
    explicit VideoPlayerGLScreenshot(const QString& videoFile, QObject *parent = nullptr);
    virtual ~VideoPlayerGLScreenshot();

    void retrieveScreenshot();

    virtual void registerNewFrame() override;

    static void playerEventCallback(const struct libvlc_event_t *p_event, void *p_data);

signals:
    void screenshotReady(const QImage& image);

protected slots:
    void videoAvailable();

protected:
    virtual void timerEvent(QTimerEvent *event) override;

    virtual bool initializeVLC() override;
    virtual bool startPlayer() override;
    virtual bool stopPlayer(bool restart) override;

    void cleanupPlayer();
    QImage extractImage();

    QString _videoFile;
    VideoPlayerGLVideo* _video;
    libvlc_media_player_t* _vlcPlayer;
    libvlc_media_t* _vlcMedia;
    int _framesReceived;
    int _status;

};

#endif // VIDEOPLAYERGLSCREENSHOT_H
